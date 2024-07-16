#include "search_server.h"

#include <cmath>

using namespace std;



SearchServer::SearchServer(const string& stop_words_text)
        : SearchServer(SplitIntoWords(stop_words_text)){
    }

void SearchServer::AddDocument(int document_id, const string& document, DocumentStatus status, const vector<int>& ratings) {
    if(document_id < 0){
        throw invalid_argument("attempt to add a document with a negative id"s);
    }
        
    if(documents_.count(document_id)){
        throw invalid_argument("attempt to add a document with the id of a previously added document"s);
    }
        
    const vector<string> words = SplitIntoWordsNoStop(document);
    const double inv_word_count = 1.0 / words.size();
    for (const string& word : words) {
        word_to_document_freqs_[word][document_id] += inv_word_count;
    }
    
    documents_.emplace(document_id, DocumentData{ComputeAverageRating(ratings), status});  
    documents_id_.push_back(document_id);
}



vector<Document> SearchServer::FindTopDocuments(const string& raw_query) const {
    return FindTopDocuments(raw_query, [](int document_id, DocumentStatus status, int rating){ return status == DocumentStatus::ACTUAL; });
    }
    
vector<Document> SearchServer::FindTopDocuments(const string& raw_query, const DocumentStatus& status) const {
    return FindTopDocuments(raw_query, [&status](int document_id, DocumentStatus document_status, int rating) {return document_status == status;});
    }

int SearchServer::GetDocumentCount() const {
        return documents_.size();
    }

tuple<vector<string>, DocumentStatus> SearchServer::MatchDocument(const string& raw_query, int document_id) const {
        const Query query = ParseQuery(raw_query);
        vector<string> matched_words;
        for (const string& word : query.plus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            if (word_to_document_freqs_.at(word).count(document_id)) {
                matched_words.push_back(word);
            }
        }
        for (const string& word : query.minus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            if (word_to_document_freqs_.at(word).count(document_id)) {
                matched_words.clear();
                break;
            }
        }
        return {matched_words, documents_.at(document_id).status};
    }

int SearchServer::GetDocumentId(int index) const{
            return documents_id_.at(index);
}

bool SearchServer::IsStopWord(const string& word) const {
        return stop_words_.count(word) > 0;
    }

vector<string> SearchServer::SplitIntoWordsNoStop(const string& text) const {
        vector<string> words;
        for (const string& word : SplitIntoWords(text)) {
            if(!IsValidWord(word)){
            throw invalid_argument("presence of invalid characters in the document being added."s);
        }
            if (!IsStopWord(word)) {
                words.push_back(word);
            }
        }
        return words;
    }

int SearchServer::ComputeAverageRating(const vector<int>& ratings) {
        if (ratings.empty()) {
            return 0;
        }
        int rating_sum = 0;
        for (const int rating : ratings) {
            rating_sum += rating;
        }
        return rating_sum / static_cast<int>(ratings.size());
    }

SearchServer::QueryWord SearchServer::ParseQueryWord(string text) const {
        bool is_minus = false;
        if(!IsValidWord(text)){
            throw invalid_argument("There are invalid characters in the words of the search query"s);
        }
        if (text[0] == '-') {
            if(text.size() == 1) {
                throw invalid_argument("The absence of text after the minus symbol in the search query"s);
            }
            is_minus = true;
            text = text.substr(1);
        }
        if(!text.empty()){
            if(text[0] == '-') {
                throw invalid_argument("The presence of more than one minus sign before the words"s);
            }
        }
        return {text, is_minus, IsStopWord(text)};
    }

SearchServer::Query SearchServer::ParseQuery(const string& text) const {
        Query query;
        for (const string& word : SplitIntoWords(text)) {
            const QueryWord query_word = ParseQueryWord(word);
            if(!query_word.is_stop) {
                if (query_word.is_minus) {
                    query.minus_words.insert(query_word.data);
                } else {
                    query.plus_words.insert(query_word.data);
                }
            }
        }
        return query;
    }

double SearchServer::ComputeWordInverseDocumentFreq(const string& word) const {
        return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
    }



bool SearchServer::IsValidWord(const string& word) {
        return none_of(word.begin(), word.end(), [](char c) {
            return c >= '\0' && c < ' ';
        });
    }
