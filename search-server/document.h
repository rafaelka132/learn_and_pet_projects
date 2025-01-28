#pragma once
//struct Document;
#include <vector>
#include <iostream>
struct Document {
    Document() = default;

    Document(int id, double relevance, int rating)
        : id(id)
        , relevance(relevance)
        , rating(rating) {
    }

    int id = 0;
    double relevance = 0.0;
    int rating = 0;
};
enum class DocumentStatus {
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED,
};

std::ostream& operator << (std::ostream& os, const std::vector<Document>& documents)
{
    for (const Document& document : documents) {
        os << "{ document_id = " << document.id << ", relevance = " << document.relevance << ", rating = " << document.rating << " }";
    }
    return os;
}