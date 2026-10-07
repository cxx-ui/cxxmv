// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// Qt employee_list example


#include "employee.hpp"
#include "employee_table_model.hpp"
#include "employee_widget.hpp"
#include "cxxmv/qt/selected_element_model.hpp"
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


class main_window: public QMainWindow {
public:
    main_window() {
        auto body = new QWidget;
        setCentralWidget(body);

        auto layout = new QHBoxLayout{body};

        employees_view_ = new QTableView;
        employees_view_->setSelectionBehavior(QAbstractItemView::SelectRows);
        employees_view_->setSelectionMode(QAbstractItemView::SingleSelection);
        employees_view_->setDragDropMode(QAbstractItemView::InternalMove);

        // overwrite mode is enabled in table view by default, it drops rows on items
        // instead of dropping them between items
        employees_view_->setDragDropOverwriteMode(false);
        employees_view_->setDropIndicatorShown(true);
        layout->addWidget(employees_view_);

        auto old_mdl = employees_view_->model();
        employees_view_->setModel(&employees_model_);
        delete old_mdl;

        auto old_sel = employees_view_->selectionModel();
        employees_view_->setSelectionModel(&sel_);
        delete old_sel;

        auto cont = new employee_widget{sel_.element()};
        layout->addWidget(cont);
    }

    ~main_window() {
        // destroying widgets before models
        delete takeCentralWidget();
    }

private:
    employee_list employees_{
        {L"John", L"Smith"},
        {L"Jane", L"Doe"},
        {L"Bob", L"Brown"},
        {L"Alice", L"White"},
        {L"Tom", L"Green"}
    };

    using employee_table_model_t = std::decay_t<decltype(employee_table_model{employees_})>;
    employee_table_model_t employees_model_{employees_};

    mv::qt::selected_element_model<employee_list> sel_{employees_, &employees_model_};

    QTableView * employees_view_;
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
