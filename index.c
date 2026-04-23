// FULL FILE final version

#include "index.h"
#include "pes.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>

IndexEntry* index_find(Index *index, const char *path) {
    for (int i = 0; i < index->count; i++) {
        if (strcmp(index->entries[i].path, path) == 0)
            return &index->entries[i];
    }
    return NULL;
}

int index_remove(Index *index, const char *path) {
    for (int i = 0; i < index->count; i++) {
        if (strcmp(index->entries[i].path, path) == 0) {
            int remaining = index->count - i - 1;
            if (remaining > 0)
                memmove(&index->entries[i], &index->entries[i + 1],
                        remaining * sizeof(IndexEntry));
            index->count--;
            return index_save(index);
        }
    }
    return -1;
}

int index_load(Index *index) {
    index->count = 0;

    FILE *f = fopen(INDEX_FILE, "r");
    if (!f) return 0;

    while (index->count < MAX_INDEX_ENTRIES) {
        IndexEntry *e = &index->entries[index->count];
        char hex[HASH_HEX_SIZE + 1];

        if (fscanf(f, "%o %64s %lu %u %[^\n]\n",
                   &e->mode, hex,
                   &e->mtime_sec,
                   &e->size,
                   e->path) != 5)
            break;

        hex_to_hash(hex, &e->hash);
        index->count++;
    }

    fclose(f);
    return 0;
}

static int cmp(const void *a, const void *b) {
    return strcmp(((IndexEntry*)a)->path, ((IndexEntry*)b)->path);
}

int index_save(const Index *index) {
    Index copy = *index;
    qsort(copy.entries, copy.count, sizeof(IndexEntry), cmp);

    FILE *f = fopen(".pes/index.tmp", "w");
    if (!f) return -1;

    for (int i = 0; i < copy.count; i++) {
        char hex[HASH_HEX_SIZE + 1];
        hash_to_hex(&copy.entries[i].hash, hex);

        fprintf(f, "%o %s %lu %u %s\n",
                copy.entries[i].mode,
                hex,
                copy.entries[i].mtime_sec,
                copy.entries[i].size,
                copy.entries[i].path);
    }

    fflush(f);
    fsync(fileno(f));
    fclose(f);

    return rename(".pes/index.tmp", INDEX_FILE);
}

// 🔥 FINAL IMPLEMENTATION
int index_add(Index *index, const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return -1;

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);

    void *data = malloc(size);
    fread(data, 1, size, f);
    fclose(f);

    ObjectID id;
    object_write(OBJ_BLOB, data, size, &id);
    free(data);

    struct stat st;
    stat(path, &st);

    IndexEntry *e = index_find(index, path);
    if (!e) e = &index->entries[index->count++];

    e->mode = st.st_mode;
    e->hash = id;
    e->mtime_sec = st.st_mtime;
    e->size = st.st_size;
    strcpy(e->path, path);

    return index_save(index);
}
