#include "../include/backend.h"

typedef struct
{
	cache_entry_t *entry;
	size_t		total_size;
	task_t	   *task;
}			curl_context_t;

static size_t
write_callback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
	curl_context_t *ctx = (curl_context_t *) userdata;

	if (ctx->task->should_stop)
	{
		printf("[BGW %d] Canceled.\n", gettid());
		return 0;
	}

	size_t		total_size = size * nmemb;

	if (ctx->entry && atomic_load(&ctx->entry->is_loading))
	{
		if (cache_append_data(ctx->entry, ptr, total_size) != 0)
		{
			fprintf(stderr, "Failed to append data to cache\n");
			return 0;
		}
		ctx->total_size += total_size;
		printf(
			   "[BGW %d] Appended %zu bytes to cache, total: %zu\n",
			   gettid(),
			   total_size,
			   ctx->total_size);
	}
	else
	{
		fprintf(stderr, "Cache entry not available or not loading\n");
		return 0;
	}

	return total_size;
}

static void
load_http_resp(uv_work_t * req)
{
	task_t	   *task = (task_t *) req->data;
	CURL	   *curl;
	CURLcode	res;
	curl_context_t ctx = {0};

	char		path[1024];

	strncpy(path, task->req->path, task->req->path_len);
	path[task->req->path_len] = '\0';

	printf("[BGW %d] Starting HTTP load for path: %s\n", gettid(), path);

	ctx.entry =
		cache_start_loading(task->backend->cache, task->key, DEFAULT_TTL);
	if (!ctx.entry)
	{
		fprintf(stderr, "[BGW] Failed to create cache entry\n");
		task->result = -1;
		return;
	}

	ctx.task = task;
	ctx.total_size = 0;
	task->entry = ctx.entry;

	curl = curl_easy_init();
	if (!curl)
	{
		fprintf(stderr, "[BGW %d] Failed to initialize CURL\n", gettid());
		cache_cancel_loading(task->backend->cache, path);
		task->result = -1;
		return;
	}

	curl_easy_setopt(curl, CURLOPT_URL, path);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ctx);
	curl_easy_setopt(curl, CURLOPT_HEADER, 1L);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 120L);

	printf("[BGW %d] CURL configured, starting request...\n", gettid());

	res = curl_easy_perform(curl);

	if (res != CURLE_OK)
	{
		fprintf(
				stderr,
				"[BGW %d] CURL failed for %s: %s\n",
				gettid(),
				path,
				curl_easy_strerror(res));
		cache_cancel_loading(task->backend->cache, path);
		task->result = -1;
	}
	else
	{
		long		http_code = 0;

		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

		printf(
			   "[BGW %d] Request completed for %s: HTTP %ld, Total bytes: %zu\n",
			   gettid(),
			   path,
			   http_code,
			   ctx.total_size);

		if (http_code == 200)
		{
			printf(
				   "[BGW %d] Successfully loaded %zu bytes into cache\n",
				   gettid(),
				   ctx.total_size);
			cache_finish_loading(ctx.entry);
			task->result = 0;
		}
		else
		{
			fprintf(stderr, "[BGW] HTTP error: %ld\n", http_code);
			cache_cancel_loading(task->backend->cache, path);
			task->result = -1;
		}
	}

	curl_easy_cleanup(curl);
	printf(
		   "[BGW %d] HTTP load completed for %s with result: %d\n",
		   gettid(),
		   path,
		   task->result);
}

static void
after_load_http_resp_cb(uv_work_t * req, int status)
{
	task_t	   *task = (task_t *) req->data;

	printf(
		   "[BGW] Background work completed, status: %d, result: %d\n",
		   status,
		   task->result);
	free_task(task);
}

void
free_task(task_t * task)
{
	if (!task)
		return;

	if (atomic_load(&task->destroy_count) == 0)
	{
		atomic_fetch_add(&task->destroy_count, 1);
		return;
	}

	if (task->raw_req)
	{
		free(task->raw_req);
		task->raw_req = NULL;
	}
	if (task->req)
	{
		free(task->req);
		task->req = NULL;
	}
	if (task->key)
	{
		free(task->key);
		task->key = NULL;
	}
	free(task);
}

int
launch_bgw(task_t * task)
{
	int			err;

	printf(
		   "[BGW] Launching background worker for path %*.s\n with a key=%s",
		   (int) task->req->path_len,
		   task->req->path,
		   task->key);

	err = uv_queue_work(
						&task->backend->loop,
						&task->work_req,
						load_http_resp,
						after_load_http_resp_cb);
	if (err != 0)
	{
		fprintf(stderr, "[BGW] Failed to queue work: %s\n", uv_strerror(err));
		return err;
	}
	return 0;
}

int
cancel_bgw(task_t * task)
{
	task->should_stop = true;
	return 0;
}
