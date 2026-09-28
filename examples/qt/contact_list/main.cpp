// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// Qt contact_list example


#include "cxxmv/ranges/transform.hpp"
#include <cxxmv/vector.hpp>
#include <cxxmv/qt/range_model.hpp>
#include <QApplication>
#include <QMainWindow>
#include <QTableView>
#include <QVBoxLayout>
#include <iostream>
#include <qtableview.h>


/// Represents contact person with first and last name
struct contact {
public:
    /// Constructs contact
    contact(std::wstring fname, std::wstring lname):
        first_name_{fname}, last_name_{lname} {}

    auto & first_name() const { return first_name_; }
    void set_first_name(std::wstring val) { first_name_ = std::move(val); }

    auto & last_name() const { return last_name_; }
    void set_last_name(std::wstring val) { last_name_ = std::move(val); }

    std::wstring full_name() const {
        auto res = first_name();
        if (!last_name().empty()) {
            res += L" " + last_name();
        }

        return res;
    }

    void set_full_name(const std::wstring & val) {
        auto pos = val.find_first_of(L" ");
        if (pos == std::wstring::npos) {
            set_first_name(val);
            set_last_name({});
        } else {
            set_first_name(val.substr(0, pos));
            set_last_name(val.substr(pos + 1));
        }
    }

    bool operator==(const contact & other) const {
        return first_name() == other.first_name() && last_name() == other.last_name();
    }

private:
    std::wstring first_name_;
    std::wstring last_name_;
};


template <mv::ranges::projectable_observable Range>
class contact_table_model: public mv::qt::range_model<Range> {
public:
    /// Table columns
    enum column {
        first_name_column,
        last_name_column,
        full_name_column,
        column_count
    };

    /// Is model read only?
    static constexpr bool is_read_only = !mv::ranges::model<Range, contact>;

    /// Constructs model with specified range of contacts and parent object
    contact_table_model(Range rng, QObject * parent = nullptr):
    mv::qt::range_model<Range>{std::move(rng), parent} {}

    /// Returns number of columns
    int columnCount(const QModelIndex & parent = {}) const override {
        return parent.isValid() ? 0 : column_count;
    }

    /// Returns data of item
    QVariant data(const QModelIndex & idx, int role = Qt::DisplayRole) const override {
        if (!idx.isValid() || (role != Qt::DisplayRole && role != Qt::EditRole)) {
            return {};
        }

        const contact & cont = std::ranges::begin(this->range())[idx.row()];
        switch (idx.column()) {
        case first_name_column:
            return QString::fromStdWString(cont.first_name());
        case last_name_column:
            return QString::fromStdWString(cont.last_name());
        case full_name_column:
            return QString::fromStdWString(cont.full_name());
        default:
            return {};
        }
    }

    /// Assigns data to item
    bool setData(const QModelIndex & idx, const QVariant & var, int role = Qt::EditRole) override {
        if constexpr (is_read_only) {
            return false;
        } else {
            if (!idx.isValid() || role != Qt::EditRole) {
                return false;
            }

            auto it = std::ranges::begin(this->range()) + idx.row();
            contact cont = *it;
            auto val = var.toString().toStdWString();

            switch (idx.column()) {
            case first_name_column:
                cont.set_first_name(std::move(val));
                break;
            case last_name_column:
                cont.set_last_name(std::move(val));
                break;
            case full_name_column:
                cont.set_full_name(val);
                break;
            default:
                return false;
            }

            // range emits after_changed signal which is converted to dataChanged
            *it = cont;
            return true;
        }
    }

    /// Returns item flags
    Qt::ItemFlags flags(const QModelIndex & idx) const override {
        if (!idx.isValid()) {
            return Qt::NoItemFlags;
        }

        Qt::ItemFlags res = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
        if constexpr (!is_read_only) {
            res |= Qt::ItemIsEditable;
        }
        return res;
    }
};


template <mv::ranges::projectable_observable Range>
contact_table_model(Range && rng) -> contact_table_model<mv::ranges::all_t<Range>>;


class main_window: public QMainWindow {
public:
    main_window() {
        auto body = new QWidget;
        setCentralWidget(body);

        auto layout = new QVBoxLayout{body};

        contacts_view_ = new QTableView;
        layout->addWidget(contacts_view_);

        contacts_model_ = std::unique_ptr<QAbstractItemModel>{new contact_table_model{contacts_}};
        auto old_mdl = contacts_view_->model();
        contacts_view_->setModel(contacts_model_.get());
        delete old_mdl;
    }

private:
    mv::vector<contact> contacts_{
        {L"John", L"Smith"},
        {L"Jane", L"Doe"},
        {L"Bob", L"Brown"},
        {L"Alice", L"White"},
        {L"Tom", L"Green"}
    };

    std::unique_ptr<QAbstractItemModel> contacts_model_;
    QTableView * contacts_view_;
};


int main(int argc, char * argv[]) {
    try {
        QApplication app{argc, argv};
        main_window wnd;
        wnd.show();
        return app.exec();
    } catch (std::exception & err) {
        std::cerr << "ERROR: " << err.what() << "\n";
    }

    return 0;
}
