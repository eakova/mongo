/**
 * Utilities for testing the analyzeShardKey command.
 */
var AnalyzeShardKeyUtil = (function() {
    /**
     * Returns true if the given key pattern contains a hashed key.
     */
    function isHashedKeyPattern(keyPattern) {
        for (let fieldName in keyPattern) {
            if (keyPattern[fieldName] == "hashed") {
                return true;
            }
        }
        return false;
    }

    /**
     * Returns true if the given key pattern is {_id: 1}.
     */
    function isIdKeyPattern(keyPattern) {
        return bsonWoCompare(keyPattern, {_id: 1}) == 0;
    }

    /**
     * Returns a set of current shard key field names, candidate shard key field names and
     * index key field names combined.
     */
    function getCombinedFieldNames(currentShardKey, candidateShardKey, indexKey) {
        const fieldNames = new Set([]);
        for (let fieldName in currentShardKey) {
            fieldNames.add(fieldName);
        }
        for (let fieldName in candidateShardKey) {
            fieldNames.add(fieldName);
        }
        for (let fieldName in indexKey) {
            fieldNames.add(fieldName);
        }
        return fieldNames;
    }

    /**
     * Sets the given field to the given value. The field name can be dotted.
     */
    function setDottedField(doc, fieldName, val) {
        let obj = doc;
        const fieldNames = fieldName.split(".");
        for (let i = 0; i < fieldNames.length; i++) {
            const fieldName = fieldNames[i];
            if (i == fieldNames.length - 1) {
                obj[fieldName] = val;
                return;
            }
            if (!doc[fieldName]) {
                obj[fieldName] = {};
            }
            obj = obj[fieldName];
        }
    }

    /**
     * Enables profiling of the given database on all the given mongods.
     */
    function enableProfiler(mongodConns, dbName) {
        mongodConns.forEach(conn => {
            conn.getDB(dbName).setProfilingLevel(2);
        });
    }

    /**
     * Disables profiling of the given database on all the given mongods.
     */
    function disableProfiler(mongodConns, dbName) {
        mongodConns.forEach(conn => {
            conn.getDB(dbName).setProfilingLevel(0);
        });
    }

    return {
        isHashedKeyPattern,
        isIdKeyPattern,
        getCombinedFieldNames,
        setDottedField,
        enableProfiler,
        disableProfiler
    };
})();
