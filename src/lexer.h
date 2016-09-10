#pragma once
#include "token.h"
#include <iterator>

bool tokenize_template(const std::string &content, tk_vec &tokens);

class tk_iterator : public std::iterator<std::forward_iterator_tag, const tk>
{
  private:
    tk_vec::const_iterator it;
    tk_vec::const_iterator end;
    const static tk end_of_input;

  public:
    tk_iterator(const tk_vec::const_iterator &it, const tk_vec::const_iterator &end)
        : it(it), end(end)
    {
    }

    tk_iterator(const tk_iterator &tk_it) : it(tk_it.it), end(tk_it.end) {}

    reference operator*() const
    {
        if (it == end)
            return end_of_input;

        return it.operator*();
    }

    pointer operator->() const
    {
        if (it == end)
            return &end_of_input;

        return it.operator->();
    }

    tk_iterator &operator++()
    {
        if (it != end)
            it.operator++();

        return *this;
    }

    tk_iterator operator++(int)
    {
        if (it != end)
            return tk_iterator(it++, end);

        return tk_iterator(it, end);
    }
};
