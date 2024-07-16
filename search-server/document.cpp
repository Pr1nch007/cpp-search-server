#include "document.h"

using namespace std;

Document::Document(int id, double relevance, int rating)
        : id(id)
        , relevance(relevance)
        , rating(rating) {
    }

ostream& operator<<(ostream& output, const Document& doc){
    output << "{ document_id = " << doc.id << ", relevance = " << doc.relevance << ", rating = " << doc.rating << " }";
    return output;
    }
