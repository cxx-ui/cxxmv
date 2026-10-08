// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// Qt line_edit_object example


#include <cxxmv/signals.hpp>
#include <cxxmv/transform.hpp>
#include <cxxmv/qt/line_edit.hpp>
#include <QApplication>
#include <QMainWindow>
#include <QFormLayout>
#include <QPushButton>
#include <QVBoxLayout>
#include <iostream>


/// Represents contact person with first and last name
class contact {
public:
    /// Constructs contact
    contact(std::wstring fname, std::wstring lname):
        first_name_{fname}, last_name_{lname} {}

    /// Contact is not copyable
    contact(const contact &) = delete;

    /// Contact is not movable
    contact(contact &&) = delete;

    auto & first_name() const { return first_name_; }

    void set_first_name(std::wstring val) {
        before_changed_();
        first_name_ = std::move(val);
        after_changed_();
    }

    auto & last_name() const { return last_name_; }

    void set_last_name(std::wstring val) {
        before_changed_();
        last_name_ = std::move(val);
        after_changed_();
    }

    std::wstring full_name() const {
        auto res = first_name_;
        if (!last_name_.empty()) {
            res += L" " + last_name_;
        }

        return res;
    }

    void set_full_name(const std::wstring & val) {
        before_changed_();

        auto pos = val.find_first_of(L" ");
        if (pos == std::wstring::npos) {
            first_name_ = val;
            last_name_.clear();
        } else {
            first_name_ = val.substr(0, pos);
            last_name_ = val.substr(pos + 1);
        }

        after_changed_();
    }

    /// Returns signal emitted before contact is changed
    auto & before_changed() const { return before_changed_; }

    /// Returns signal emitted after contact is changed
    auto & after_changed() const { return after_changed_; }

private:
    std::wstring first_name_;
    std::wstring last_name_;

    mutable mv::signal<void ()> before_changed_;    ///< Before changed signal
    mutable mv::signal<void ()> after_changed_;     ///< After changed signal
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

        auto get_full_name = [](const contact & c) { return c.full_name(); };
        auto set_full_name = [](contact & c, const std::wstring & f_name) { c.set_full_name(f_name); };
        auto full_name = new mv::qt::line_edit{mdl_ | mv::transform(get_full_name, set_full_name)};
        layout->addRow("Full name:", full_name);
    }

private:
    contact mdl_{L"Jonh", L"Smith"};
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
