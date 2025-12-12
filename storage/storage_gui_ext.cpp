
    // GUI Helper: Scan directory for tables
    vector<string> StorageEngine::getTableNames() const {
        vector<string> tables;
        if (!fs::exists(storageDirectory)) return tables;

        for (const auto& entry : fs::directory_iterator(storageDirectory)) {
            if (entry.path().extension() == ".meta") {
                // filename without extension is table name
                tables.push_back(entry.path().stem().string());
            }
        }
        return tables;
    }

    bool StorageEngine::tableExists(const string& tableName) const {
         return fs::exists(tableMetaPath(tableName));
    }
