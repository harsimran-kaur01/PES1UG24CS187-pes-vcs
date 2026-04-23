int object_read(const ObjectID *id, ObjectType *type_out, void **data_out, size_t *len_out) {
    if (!id || !type_out || !data_out || !len_out) return -1;

    char path[512];
    object_path(id, path, sizeof(path));

    FILE *f = fopen(path, "rb");
    if (!f) return -1;

    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    rewind(f);

    void *buffer = malloc(size);
    if (!buffer) { fclose(f); return -1; }

    if (fread(buffer, 1, size, f) != size) {
        fclose(f);
        free(buffer);
        return -1;
    }
    fclose(f);

    ObjectID check;
    compute_hash(buffer, size, &check);

    if (memcmp(&check, id, sizeof(ObjectID)) != 0) {
        free(buffer);
        return -1;
    }

    char *nul = memchr(buffer, '\0', size);
    if (!nul) { free(buffer); return -1; }

    char type[10];
    size_t data_len;

    sscanf(buffer, "%s %zu", type, &data_len);

    if (strcmp(type, "blob") == 0) *type_out = OBJ_BLOB;
    else if (strcmp(type, "tree") == 0) *type_out = OBJ_TREE;
    else *type_out = OBJ_COMMIT;

    *data_out = malloc(data_len);
    if (!*data_out) { free(buffer); return -1; }

    memcpy(*data_out, nul + 1, data_len);
    *len_out = data_len;

    free(buffer);
    return 0;
}
