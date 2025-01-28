#pragma once
#include <vector>
#include "document.h"
template <typename Container>
auto Paginate(const Container& c, size_t page_size) {
    return Paginator(begin(c), end(c), page_size);
}

template <typename Iterator>
class Paginator {
    // тело класса
public:
    Paginator(Iterator begin, Iterator end, size_t page_size) {
        while (begin < end) {
            std::vector<Document> page_;
            for (size_t page_size_in = 0; page_size_in < page_size; ++page_size_in) {
                if (begin < end) {
                    page_.push_back(*begin);
                    ++begin;
                }
            }
            pages_.push_back(page_);
        }

    }
    auto begin() const {
        return pages_.begin();
    }
    auto end() const {
        return pages_.end();
    }

    std::vector<std::vector<Document>> pages_;
};