#include <iostream>
#include <string>

namespace macros
{
template <typename O, typename T0> void fact(O &o, T0 vsym_n);

template <typename O, typename T0, typename T1, typename T2>
void input(O &o, T0 vsym_name, T1 vsym_value, T2 vsym_type);

template <typename O, typename T0, typename T1>
void print_users(O &o, T0 vsym_users, T1 vsym_active);

template <typename O, typename T0> void fact(O &o, T0 vsym_n)
{
    o << u8R"content"""(
    )content"""";
    if ((vsym_n == 1)) {
        o << u8R"content"""(
        <span>)content"""";
        o << vsym_n;
        o << u8R"content"""(</span>
    )content"""";
    } else {
        o << u8R"content"""(
        <span>)content"""";
        o << vsym_n;
        o << u8R"content"""( * )content"""";
        macros::fact(o, (vsym_n - 1));
        o << u8R"content"""(</span>
    )content"""";
    }
    o << u8R"content"""(
)content"""";
}

template <typename O, typename T0, typename T1, typename T2>
void input(O &o, T0 vsym_name, T1 vsym_value, T2 vsym_type)
{
    o << u8R"content"""(
    <input type=")content"""";
    o << vsym_type;
    o << u8R"content"""(" name=")content"""";
    o << vsym_name;
    o << u8R"content"""(" value=")content"""";
    o << vsym_value;
    o << u8R"content"""(">
)content"""";
}

template <typename O, typename T0, typename T1>
void print_users(O &o, T0 vsym_users, T1 vsym_active)
{
    o << u8R"content"""(
    <ol>

    )content"""";
    {
        auto vsym_label = std::string("Deactivate");
        o << u8R"content"""(
    )content"""";
        if (!(vsym_active)) {
            o << u8R"content"""(
        )content"""";
            {
                vsym_label = std::string("Activate");
                o << u8R"content"""(
    )content"""";
            }
        } else {
        }
        o << u8R"content"""(

    )content"""";
        for (const auto &vsym_user : vsym_users) {
            if (((vsym_user.active) == vsym_active)) {
                o << u8R"content"""(

        <li style="margin: 10px 0;">
            )content"""";
                o << (vsym_user.firstname);
                o << u8R"content"""( )content"""";
                o << (vsym_user.lastname);
                o << u8R"content"""( <br>
            <button style="margin: 3px 0" type="submit" value=")content"""";
                o << (vsym_user.id);
                o << u8R"content"""(" name=")content"""";
                o << vsym_label;
                o << u8R"content"""("> )content"""";
                o << vsym_label;
                o << u8R"content"""( </button>
        </li>

    )content"""";
            }
        }
        o << u8R"content"""(

    </ol>
)content"""";
    }
}
}

template <typename OS, typename N0, typename N1>
void render_template(OS &o, N0 vsym_history, N1 vsym_users)
{
    o << u8R"content"""(<html>
<head>

</head>
<body>

)content"""";
    o << u8R"content"""(

)content"""";
    o << u8R"content"""(


)content"""";
    o << u8R"content"""(


<form>
    )content"""";
    macros::input(o, std::string("a"), std::string(""), std::string("text"));
    o << u8R"content"""(
    )content"""";
    macros::input(o, std::string("b"), std::string(""), std::string("text"));
    o << u8R"content"""(
    )content"""";
    macros::input(o, std::string("c"), std::string("Ok"), std::string("submit"));
    o << u8R"content"""(
</form>


<ol style="list-style-type: none;">
    )content"""";
    for (const auto &vsym_entry : vsym_history) {
        if (true) {
            o << u8R"content"""(
        )content"""";
            {
                auto vsym_a = (vsym_entry.first);
                o << u8R"content"""(
        )content"""";
                {
                    auto vsym_b = (vsym_entry.second);
                    o << u8R"content"""(

        <li style="margin: 10px 0;">
            )content"""";
                    if (((vsym_a * vsym_b) > 0)) {
                        o << u8R"content"""(
                <span style="color: green;">
            )content"""";
                    } else {
                        o << u8R"content"""(
                <span style="color: red;">
            )content"""";
                    }
                    o << u8R"content"""(

            )content"""";
                    o << vsym_a;
                    o << u8R"content"""( * )content"""";
                    o << vsym_b;
                    o << u8R"content"""( = )content"""";
                    o << (vsym_a * vsym_b);
                    o << u8R"content"""(

            </span>
            <br/>
            )content"""";
                    o << vsym_a;
                    o << u8R"content"""(² + )content"""";
                    o << vsym_b;
                    o << u8R"content"""(² = )content"""";
                    o << ((vsym_a * vsym_a) + (vsym_b * vsym_b));
                    o << u8R"content"""(
            <br/>
            )content"""";
                    macros::fact(o, vsym_a);
                    o << u8R"content"""(
        </li>
    
    )content"""";
                }
            }
        }
    }
    o << u8R"content"""(
</ol>

<form>
<h3>Active Users</h3>
)content"""";
    macros::print_users(o, vsym_users, true);
    o << u8R"content"""(

<h3>Inactive Users</h3>
)content"""";
    macros::print_users(o, vsym_users, false);
    o << u8R"content"""(
</form>

</body>
</html>

)content"""";
}
