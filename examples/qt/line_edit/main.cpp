// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// Qt line_edit example


#include <cxxmv/basic_model.hpp>
#include <cxxmv/transform.hpp>
#include <cxxmv/qt/line_edit.hpp>
#include <QApplication>
#include <QMainWindow>
#include <QFormLayout>
#include <QPushButton>
#include <QVBoxLayout>
#include <iostream>
#include <sstream>


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

        auto get_first_name = [](const contact & c) { return c.first_name(); };
        auto set_first_name = [](contact & c, const std::wstring & f_name) { c.set_first_name(f_name); };
        auto f_name = new mv::qt::line_edit{mdl_ | mv::transform(get_first_name, set_first_name)};
        layout->addRow("First name:", f_name);

        auto get_last_name = [](const contact & c) { return c.last_name(); };
        auto set_last_name = [](contact & c, const std::wstring & l_name) { c.set_last_name(l_name); };
        auto l_name = new mv::qt::line_edit{mdl_ | mv::transform(get_last_name, set_last_name)};
        layout->addRow("Last name:", l_name);

        auto get_full_name = [](const contact & c) {
            auto res = c.first_name();
            if (!c.last_name().empty()) {
                res += L" " + c.last_name();
            }

            return res;
        };
        
        auto set_full_name = [](contact & c, const std::wstring & full_name) {
            auto pos = full_name.find_first_of(L" ");
            if (pos == std::wstring::npos) {
                c.set_first_name(full_name);
                c.set_last_name({});
            } else {
                c.set_first_name(full_name.substr(0, pos));
                c.set_last_name(full_name.substr(pos + 1));
            }
        };

        auto xview = mdl_ | mv::transform(get_full_name, set_full_name);
        auto full_name = new mv::qt::line_edit{xview};
        layout->addRow("Full name:", full_name);
    }

private:
    mv::basic_model<contact> mdl_{L"Jonh", L"Smith"};
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
