#include "search_server.h"
#include "request_queue.h"
#include "document.h"
#include "string_processing.h"
#include "paginator.h"
#include "read_input_functions.h"

#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <set>

using namespace std;

void PrintHelp() {
    cout << "Available commands:\n"
         << "  add <id> <status> <rating1> <rating2> ... <ratingN> -- <text> : Add a document\n"
         << "  find : Search for documents\n"
         << "  count : Show document count\n"
         << "  exit : Exit the program\n";
}

DocumentStatus ParseSingleStatus(const string& status_str) {
    if (status_str == "ACTUAL") return DocumentStatus::ACTUAL;
    if (status_str == "IRRELEVANT") return DocumentStatus::IRRELEVANT;
    if (status_str == "BANNED") return DocumentStatus::BANNED;
    if (status_str == "REMOVED") return DocumentStatus::REMOVED;
    throw invalid_argument("Invalid status: " + status_str);
}

set<DocumentStatus> ParseStatus(const string& status_input) {
    set<DocumentStatus> statuses;
    if (status_input == "ALL") {
        return {DocumentStatus::ACTUAL, DocumentStatus::IRRELEVANT, 
                DocumentStatus::BANNED, DocumentStatus::REMOVED};
    }
    
    vector<string> status_words = SplitIntoWords(status_input);
    for (const string& status_str : status_words) {
        statuses.insert(ParseSingleStatus(status_str));
    }
    return statuses;
}

vector<int> ParseRatings(const vector<string>& words, size_t start_index, size_t end_index) {
    vector<int> ratings;
    for (size_t i = start_index; i < end_index; ++i) {
        try {
            ratings.push_back(stoi(words[i]));
        } catch (const invalid_argument&) {
            throw invalid_argument("Invalid rating: " + words[i]);
        }
    }
    return ratings;
}

void AddDocument(SearchServer& server) {
    cout << "Enter: id status rating1 rating2 ... ratingN -- document text\n";
    string input = ReadLine();
    vector<string> parts = SplitIntoWords(input);

    auto separator_it = find(parts.begin(), parts.end(), "--");
    if (separator_it == parts.end() || separator_it < parts.begin() + 2) {
        throw invalid_argument("Invalid add command format: missing '--' or insufficient arguments before it");
    }

    size_t separator_index = distance(parts.begin(), separator_it);

    if (separator_index < 2) {
        throw invalid_argument("Invalid add command format: missing id or status");
    }

    int id;
    try {
        id = stoi(parts[0]);
    } catch (const invalid_argument&) {
        throw invalid_argument("Invalid document ID: " + parts[0]);
    }

    DocumentStatus status = ParseSingleStatus(parts[1]);
    vector<int> ratings = ParseRatings(parts, 2, separator_index);

    string document_text;
    for (size_t i = separator_index + 1; i < parts.size(); ++i) {
        document_text += parts[i] + (i < parts.size() - 1 ? " " : "");
    }
    if (document_text.empty()) {
        throw invalid_argument("Document text cannot be empty");
    }

    server.AddDocument(id, document_text, status, ratings);
    cout << "Document added successfully\n";
}

void FindDocuments(SearchServer& server, RequestQueue& request_queue) {
    cout << "Enter status (ACTUAL, IRRELEVANT, BANNED, REMOVED, or multiple statuses separated by spaces, or ALL):\n";
    string status_input = ReadLine();
    set<DocumentStatus> statuses;
    try {
        statuses = ParseStatus(status_input);
    } catch (const invalid_argument& e) {
        cout << "Error: " << e.what() << "\n";
        return;
    }

    cout << "Enter search query:\n";
    string query = ReadLine();
    
    vector<Document> documents = request_queue.AddFindRequest(query, [&statuses](int, DocumentStatus document_status, int) {
        return statuses.count(document_status) > 0;
    });
    
    if (documents.empty()) {
        cout << "No documents found\n";
        return;
    }

    const size_t page_size = 2;
    auto paginated_results = Paginate(documents, page_size);
    
    cout << "Found " << documents.size() << " documents:\n";
    size_t page_number = 1;
    for (const auto& page : paginated_results) {
        cout << "Page " << page_number++ << ":\n";
        cout << page << "\n";
    }
}

int main() {
    try {
        SearchServer search_server("and in at"s);
        RequestQueue request_queue(search_server);

        cout << "Search Engine started. Type 'help' for commands.\n";

        while (true) {
            cout << "> ";
            string command = ReadLine();
            if (command.empty()) continue;

            try {
                if (command == "help") {
                    PrintHelp();
                } else if (command == "add") {
                    AddDocument(search_server);
                } else if (command == "find") {
                    FindDocuments(search_server, request_queue);
                } else if (command == "count") {
                    cout << "Total documents: " << search_server.GetDocumentCount() << "\n";
                } else if (command == "exit") {
                    cout << "Exiting program\n";
                    break;
                } else {
                    cout << "Unknown command. Type 'help' for available commands.\n";
                }
            } catch (const exception& e) {
                cout << "Error: " << e.what() << "\n";
            }
        }
    } catch (const exception& e) {
        cout << "Initialization error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
