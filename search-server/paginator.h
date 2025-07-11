#pragma once
#include "document.h"

#include <algorithm>
#include <string>
#include <iostream>
#include <vector>

template <typename Iterator>
    class IteratorRange {
    public:
        IteratorRange(Iterator first, Iterator last):
        first_(first), last_(last), size_(distance(first_, last_))
        {
        }
        Iterator begin() const{
        return first_;
        }
        Iterator end() const{
        return last_;
        }
        size_t size() const {
        return size_;
    }
    private:
        Iterator first_; 
        Iterator last_;
        size_t size_;
    };

    template <typename Iterator>
class Paginator {
public:
    Paginator(Iterator begin, Iterator end, size_t page_size) {
        for (size_t left = distance(begin, end); left > 0;) {
            const size_t current_page_size = std::min(page_size, left);
            const Iterator current_page_end = next(begin, current_page_size);
            pages_.push_back({begin, current_page_end});

            left -= current_page_size;
            begin = current_page_end;
        }
    }

    auto begin() const {
        return pages_.begin();
    }

    auto end() const {
        return pages_.end();
    }

    size_t size() const {
        return pages_.size();
    }

private:
    std::vector<IteratorRange<Iterator>> pages_;
};
    
    template <typename Iterator>
    std::ostream& operator<<(std::ostream& output, const IteratorRange<Iterator>& page){
        for(auto i = page.begin(); i != page.end(); ++i){
            output << *i;
        }
        return output;
    }
