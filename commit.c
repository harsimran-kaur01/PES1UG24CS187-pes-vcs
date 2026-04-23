// commit.c

#include "commit.h"
#include "index.h"
#include "tree.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>

// Forward declarations
int object_write(ObjectType type, const void *data, size_t len, ObjectID *id_out);
int object_read(const ObjectID *id, ObjectType *type_out, void **data_out, size_t *len_out);

// ─── PROVIDED ────────────────────────────────────────────────────────────────

int commit_parse(const void *data, size_t len, Commit *commit_out) {
    (void)len;
    const char *p = (const char *)data;
    char hex[HASH_HEX_SIZE + 1];

    if (sscanf(p, "tree %64s\n", hex) != 1) return -1;
    if (hex_to_hash(hex, &commit_out->tree) != 0) return -1;
    p = strchr(p, '\n') + 1;

    if (strncmp(p, "parent ", 7) == 0) {
        if (sscanf(p, "parent %64s\n", hex) != 1) return -1;
        if (hex_to_hash(hex, &commit_out->parent) != 0) return -1;
        commit_out->has_parent = 1;
        p = strchr(p, '\n') + 1;
    } else {
        commit_out->has_parent = 0;
    }

    char author_buf[256];
    uint64_t ts;

    if (sscanf(p, "author %255[^\n]\n", author_buf) != 1) return -1;

    char *last_space = strrchr(author_buf, ' ');
    if (!last_space) return -1;

    ts = (uint64_t)strtoull(last_space + 1, NULL, 10);
    *last_space = '\0';

    snprintf(commit_out->author, sizeof(commit_out->author), "%s", author_buf);
    commit_out->timestamp = ts;

    p = strchr(p, '\n') + 1;
    p = strchr(p, '\n') + 1;
    p = strchr(p, '\n') + 1;

    snprintf(commit_out->message, sizeof(commit_out->message), "%s", p);
    return 0;
}

int commit_serialize(const Commit *commit, void **data_out, size_t *len_out) {
    char tree_hex[HASH_HEX_SIZE + 1];
    char parent_hex[HASH_HEX_SIZE + 1];

    hash_to_hex(&commit->tree, tree_hex);

    char buf[8192];
    int n = 0;

    n += snprintf(buf + n, sizeof(buf) - n, "tree %s\n", tree_hex);

    if (commit->has_parent) {
        hash_to_hex(&commit->parent, parent_hex);
        n += snprintf(buf + n, sizeof(buf) - n, "parent %s\n", parent_hex);
    }

    n += snprintf(buf + n, sizeof(buf) - n,
                  "author %s %" PRIu64 "\n"
                  "committer %s %" PRIu64 "\n\n%s",
                  commit->author, commit->timestamp,
                  commit->author, commit->timestamp,
                  commit->message);

    *data_out = malloc(n + 1);
    if (!*data_out) return -1;

    memcpy(*data_out, buf, n + 1);
    *len_out = (size_t)n;
    return 0;
}

// ─── TODO IMPLEMENTATION (STEP 1) ────────────────────────────────────────────

int commit_create(const char *message, ObjectID *commit_id_out) {
    Commit c;
    memset(&c, 0, sizeof(c));

    // Step 1: build tree
    if (tree_from_index(&c.tree) != 0)
        return -1;

    (void)message;
    (void)commit_id_out;
    return 0;
}
