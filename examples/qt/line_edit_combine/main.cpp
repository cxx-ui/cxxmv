// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// Qt line_edit example with combining models


#include <cxxmv/basic_model.hpp>
#include <cxxmv/operators.hpp>
#include <cxxmv/transform.hpp>
#include <cxxmv/string.hpp>
#include <cxxmv/qt/line_edit.hpp>
#include <QApplication>
#include <QMainWindow>
#include <QFormLayout>
#include <iostream>


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

    bool operator==(const contact & other) const {
        return first_name() == other.first_name() && last_name() == other.last_name();
    }

private:
    std::wstring first_name_;
    std::wstring last_name_;
};


class main_window: public QMainWindow {
public:
    main_window() {
        auto body = new QWidget;
        setCentralWidget(body);

        auto * layout = new QFormLayout{body};

        layout->addRow("First name:", new mv::qt::line_edit{first_name_});
        layout->addRow("Last name:", new mv::qt::line_edit{last_name_});
        layout->addRow("Full name:", new mv::qt::line_edit{first_name_ + L" " + last_name_});
    }

private:
    mv::wstring first_name_{L"Jonh"};
    mv::wstring last_name_{L"Smith"};
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
