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
#include <vector>
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

    /// Does range support moving of contacts?
    static constexpr bool supports_move = mv::ranges::model_with_move<Range, contact>;

    /// Does range support inserting of contacts?
    static constexpr bool supports_insert = mv::ranges::model_with_insert<Range, contact>;

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
            // display role is assigned by setItemData when dropped rows are inserted
            if (!idx.isValid() || (role != Qt::EditRole && role != Qt::DisplayRole)) {
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

            it.mut() = cont;
            return true;
        }
    }

    /// Returns item flags
    Qt::ItemFlags flags(const QModelIndex & idx) const override {
        if (!idx.isValid()) {
            // root item support drag and drop
            return supports_move ? Qt::ItemIsDropEnabled : Qt::NoItemFlags;
        }

        Qt::ItemFlags res = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
        if constexpr (!is_read_only) {
            res |= Qt::ItemIsEditable;
        }

        if constexpr (supports_move) {
            res |= Qt::ItemIsDragEnabled;
        }

        return res;
    }

    /// Inserts empty contacts at specified row
    bool insertRows(int row, int count, const QModelIndex & parent = {}) override {
        if constexpr (!supports_insert) {
            return false;
        } else {
            if (parent.isValid() || row < 0 || row > this->rowCount() || count <= 0) {
                return false;
            }

            std::vector<contact> vals(count, contact{{}, {}});
            auto pos = std::ranges::begin(this->range()) + row;
            this->range().insert(pos, vals.begin(), vals.end());
            return true;
        }
    }

    /// Returns supported drop actions
    Qt::DropActions supportedDropActions() const override {
        return supports_move ? Qt::MoveAction : Qt::IgnoreAction;
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
        contacts_view_->setSelectionBehavior(QAbstractItemView::SelectRows);
        contacts_view_->setSelectionMode(QAbstractItemView::SingleSelection);
        contacts_view_->setDragDropMode(QAbstractItemView::InternalMove);

        // overwrite mode is enabled in table view by default, it drops rows on items
        // instead of dropping them between items
        contacts_view_->setDragDropOverwriteMode(false);
        contacts_view_->setDropIndicatorShown(true);
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
