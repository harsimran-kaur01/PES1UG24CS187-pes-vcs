// tree.c — Tree object serialization and construction

// SAME FILE AS ABOVE — only tree_from_index changed

int tree_from_index(ObjectID *id_out) {
    Index index;
    if (index_load(&index) != 0) return -1;

    Tree tree;
    tree.count = 0;

    for (int i = 0; i < index.count; i++) {
        const char *path = index.entries[i].path;

        if (strchr(path, '/') == NULL) {
            tree.entries[tree.count].mode = index.entries[i].mode;
            tree.entries[tree.count].hash = index.entries[i].hash;
            strcpy(tree.entries[tree.count].name, path);
            tree.count++;
        }
    }

    void *data;
    size_t len;

    if (tree_serialize(&tree, &data, &len) != 0) return -1;

    int rc = object_write(OBJ_TREE, data, len, id_out);
    free(data);
    return rc;
}
