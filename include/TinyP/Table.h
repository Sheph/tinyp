#ifndef _TINYP_TABLE_H_
#define _TINYP_TABLE_H_

#include "TinyP/Error.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

namespace TinyP {
    // Value type for table cells
    struct Value {
        std::string data;
        bool isNumeric;

        Value() : isNumeric(false) {}
        Value(const std::string& d, bool num) : data(d), isNumeric(num) {}
        Value(double d) : data(std::to_string(d)), isNumeric(true) {}
        Value(const char* s) : data(s), isNumeric(false) {}

        double asDouble() const { return std::atof(data.c_str()); }
        const std::string& asString() const { return data; }
    };

    // Column definition
    struct Column {
        std::string name;
        bool isNumeric;

        Column() : isNumeric(false) {}
        Column(const std::string& n, bool num = false) : name(n), isNumeric(num) {}
    };

    // A row in the table
    using Row = std::vector<Value>;

    // In-memory table
    class Table
    {
    public:
        Table() = default;
        explicit Table(const std::string& name) : name_(name) {}

        inline const std::string& name() const { return name_; }

        // Add a column
        void addColumn(const std::string& name, bool isNumeric = false) {
            columns_.push_back(Column(name, isNumeric));
            columnIndex_[name] = columns_.size() - 1;
        }

        // Add a row
        void addRow(const Row& row) {
            rows_.push_back(row);
        }

        // Get column index by name
        Result<size_t> getColumnIndex(const std::string& name) const {
            auto it = columnIndex_.find(name);
            if (it == columnIndex_.end()) {
                return Error("unknown column: " + name, 0, 0);
            }
            return it->second;
        }

        // Get column names
        std::vector<std::string> getColumnNames() const {
            std::vector<std::string> names;
            for (const auto& col : columns_) {
                names.push_back(col.name);
            }
            return names;
        }

        inline const std::vector<Column>& columns() const { return columns_; }
        inline const std::vector<Row>& rows() const { return rows_; }
        inline size_t columnCount() const { return columns_.size(); }
        inline size_t rowCount() const { return rows_.size(); }

    private:
        std::string name_;
        std::vector<Column> columns_;
        std::vector<Row> rows_;
        std::unordered_map<std::string, size_t> columnIndex_;
    };

    using TablePtr = std::shared_ptr<Table>;
}

#endif
