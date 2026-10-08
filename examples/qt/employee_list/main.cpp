// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// Qt employee_list example


#include "employee.hpp"
#include "employee_list_widget.hpp"
#include "position.hpp"
#include "position_list_widget.hpp"
#include <QApplication>
#include <QHBoxLayout>
#include <QMainWindow>
#include <iostream>


class main_window: public QMainWindow {
public:
    main_window() {
        positions_.emplace_back(L"Manager", employees_, &employees_[1]);
        positions_.emplace_back(L"Developer", employees_, &employees_[0]);
        positions_.emplace_back(L"Tester", employees_, &employees_[3]);

        auto body = new QWidget;
        setCentralWidget(body);

        auto layout = new QHBoxLayout{body};
        layout->addWidget(new employee_list_widget{employees_});
        layout->addWidget(new position_list_widget{positions_, employees_});
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

    position_list positions_;
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
