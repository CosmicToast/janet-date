#include "date.h"

#define JD_STRFTIME_CHUNK 64
JanetBuffer *strftime_buffer(const char *format, const struct tm *tm, JanetBuffer *buffer) {
	if (!buffer) buffer = janet_buffer(0);
	size_t offset  = buffer->count;
	size_t written = 0;
	do {
		janet_buffer_extra(buffer, JD_STRFTIME_CHUNK);
		written = strftime((char*)buffer->data + offset, buffer->capacity - offset, format, tm);
	} while (!written);
	buffer->count = written + offset; // does not include \0, but we don't want it anyway
	return buffer;
}
