#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;

string ReadLine() {
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result = 0;
    cin >> result;
    ReadLine();
    return result;
}

vector<string> SplitIntoWords(const string& text) {
    vector<string> words;
    string word;
    
    for (const char c : text) {
        if (c == ' ') {
            if (!word.empty()) {
                words.push_back(word);
                word.clear();
            }
        } else {
            word += c;
        }
    }
    
    if (!word.empty()) {
        words.push_back(word);
    }

    return words;
}

struct Document {
    int id;
    double relevance;
};

class SearchServer {
public:
    void AddCount(const int& number){
        document_count_ = number;
    }
    
    void SetStopWords(const string& text) {
        for (const string& word : SplitIntoWords(text)) {
            stop_words_.insert(word);
        }
    }

    void AddDocument(int document_id, const string& document) {
        const vector<string> words = SplitIntoWordsNoStop(document);
        double TF_one_word = 1.0/words.size();

        for (const string& word : words){
            documents_[word][document_id] += TF_one_word;
        }
    }

    vector<Document> FindTopDocuments(const string& raw_query) const {
        const Query query_words = ParseQuery(raw_query);
        auto matched_documents = FindAllDocuments(query_words);

        sort(matched_documents.begin(), matched_documents.end(),
             [](const Document& lhs, const Document& rhs) {
                 return lhs.relevance > rhs.relevance;
             });

        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }
        return matched_documents;
    }

private:
    map <string, map<int,double>> documents_;

    set<string> stop_words_;

    int document_count_ = 0;
    
    bool IsStopWord(const string& word) const {
        return stop_words_.count(word) > 0;
    }

    vector<string> SplitIntoWordsNoStop(const string& text) const {
        vector<string> words;
        
        for (const string& word : SplitIntoWords(text)) {
            bool word_is_stop_words = IsStopWord(word);
            
            if (!word_is_stop_words) {
                words.push_back(word);
            }
        }
        return words;
    }

struct QueryWord {
        string data;
        bool is_minus;
        bool is_stop;
    };
 
    struct Query {
        set<string> plus_words;
        set<string> minus_words;
    };

    QueryWord ParseQueryWord(string text) const {
        bool is_minus = false;
        
        if (text[0] == '-') {
            is_minus = true;
            text = text.substr(1);
        }
        return {text, is_minus, IsStopWord(text)};
    }

    Query ParseQuery(const string& text) const {
        Query query;

        for (const string& word : SplitIntoWords(text)) {
            const QueryWord query_word = ParseQueryWord(word);
            bool query_word_is_stop_word = query_word.is_stop;
            bool query_word_is_minus_word = query_word.is_minus;

            if (!query_word_is_stop_word) {
                if (query_word_is_minus_word) {
                    query.minus_words.insert(query_word.data);
                } else {
                    query.plus_words.insert(query_word.data);
                }
            }
        }
        return query;
    }

    double IDF(const string& word) const {
        double IDF_word = log(document_count_*1.0/documents_.at(word).size());
        return IDF_word;
    }

    vector<Document> FindAllDocuments(const Query& query_words) const { 
        map <int, double> id_relev;
        
        for(const string& word : query_words.plus_words){
            if(documents_.count(word) == 0){
                continue;
            }
            
            double IDF_word = IDF(word);
            
            for(const auto& [id,TF] : documents_.at(word)){
                id_relev[id] += TF*IDF_word;
                }
            }
        
        for(const string& word : query_words.minus_words){
            if(documents_.count(word) == 0){
                continue;
            }
            
            for(const auto& [id,TF] : documents_.at(word)){
                    id_relev.erase(id);
                }
            }
        
        vector<Document> matched_documents;
        
        for(const auto& [key,value] : id_relev){
             matched_documents.push_back({key,value});
        }
        return matched_documents;
    }
};

SearchServer CreateSearchServer() {
    SearchServer search_server;
    search_server.SetStopWords(ReadLine());
    
    const int document_count = ReadLineWithNumber();
    search_server.AddCount(document_count);
    
    for (int document_id = 0; document_id < document_count; ++document_id) {
        search_server.AddDocument(document_id, ReadLine());
    }

    return search_server;
}

int main() {
    const SearchServer search_server = CreateSearchServer();
    const string query = ReadLine();
    
    for (const auto& [document_id, relevance] : search_server.FindTopDocuments(query)) {
        cout << "{ document_id = "s << document_id << ", "
             << "relevance = "s << relevance << " }"s << endl;
    }
}