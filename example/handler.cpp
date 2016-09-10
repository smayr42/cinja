#include "template.h"

#include <boost/lexical_cast.hpp>
#include <pion/http/plugin_service.hpp>
#include <pion/http/response_writer.hpp>
#include <stdexcept>
#include <vector>

using namespace pion;
using namespace pion::http;
using namespace pion::tcp;

struct User {
    std::string firstname;
    std::string lastname;
    bool active;
    int id;
};

static std::vector<User> users{
    {"Frist1", "Last1", true, 0}, {"Frist2", "Last2", true, 1}, {"Frist3", "Last3", true, 2}};

static std::vector<std::pair<int, int>> history;

template <typename T>
T cast_param(request_ptr &http_request_ptr, const std::string &param, const T &def = T())
{
    auto queries = http_request_ptr->get_queries();
    auto it = queries.find(param);

    if (it == queries.end())
        throw std::runtime_error("param not found");

    return boost::lexical_cast<T>(it->second);
}

class HttpService : public plugin_service
{
  public:
    virtual void operator()(request_ptr &req, connection_ptr &conn)
    {
        response_writer_ptr writer(
            response_writer::create(conn, *req, std::bind(&connection::finish, conn)));

        try {
            auto a = cast_param<int>(req, "a");
            auto b = cast_param<int>(req, "b");
            history.push_back(std::make_pair(a, b));
        } catch (...) {
        }

        auto range = req->get_queries().equal_range("Deactivate");
        for (auto it = range.first; it != range.second; ++it) {
            auto id = boost::lexical_cast<size_t>(it->second);
            if (id < users.size())
                users[id].active = false;
        }

        range = req->get_queries().equal_range("Activate");
        for (auto it = range.first; it != range.second; ++it) {
            auto id = boost::lexical_cast<size_t>(it->second);
            if (id < users.size())
                users[id].active = true;
        }

        writer->get_response().set_content_type(http::types::CONTENT_TYPE_HTML + "; charset=utf-8");
        render_template(writer, history, users);
        writer->send();
    }
};

extern "C" PION_PLUGIN HttpService *pion_create_example(void) { return new HttpService(); }
extern "C" PION_PLUGIN void pion_destroy_example(HttpService *service_ptr) { delete service_ptr; }
