//
// Created by KHS0 on 8/27/2017.
//

#ifndef STREAMUNLIMITED_FUNC_H
#define STREAMUNLIMITED_FUNC_H

block_t* sendHTTPRequest(goalbit_t_0 *p_goalbit, const char *psz_request_url, const int max_size, const int i_timeout);
void StartExecution(goalbit_t_0 *p_this);

template <typename T> array_t<T> *array_new();

template <typename T>
void array_append(array_t<T> *p_array, T** p_elem);

template <typename T>
T* array_item_at_index(array_t<T> *p_array, int i_index);

int hash_contains_key(const hash_t_0 *p_dict, const char *psz_key);
void freeSegment(hls_segment_t *p_segment);
void Push_Value(generic_queue_t_0 *queue, generic_event_t_0 event);
size_t decode_list(const char *b, size_t len, const char *keylist);
size_t decode_rev(const char *b, size_t len, const char *keylist);
block_t* block_Duplicate(block_t *p_block);
size_t curl_write(void *ptr, size_t size, size_t nmemb, block_t *p_result);
hls_segment_t* newSegment(const int id, const float duration, const uint64_t size, const uint64_t bw, const char *url, const char *desc, const bool discontinuity, const char *date_time);
void mapStream(const hls_stream_t *p_stream_src, hls_stream_t *p_stream_dst);
hls_segment_t * getSegment(const hls_stream_t *hls, const int index);
hls_segment_t * copySegment(const hls_segment_t *p_segment);
hls_stream_t* newStream(const int id, const uint64_t bw, const char *uri);

template <typename T>
T* array_item_at_index(array_t<T> *p_array, int i_index);

bool _bittest(void *pInt, unsigned int v45);
int __ROL4__(unsigned int i, int i1);
void msleep(mtime_t delay);
void print_dbg(const char* a1, const char *a2, ...);

#endif //STREAMUNLIMITED_FUNC_H
