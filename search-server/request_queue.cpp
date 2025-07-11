#include "request_queue.h"

using namespace std;

RequestQueue::RequestQueue(const SearchServer& search_server)
        :search_server_(search_server)
    {
    }

    

    vector<Document> RequestQueue::AddFindRequest(const string& raw_query, DocumentStatus status) {
        return RequestQueue::AddFindRequest(raw_query, [status](int document_id, DocumentStatus document_status, int rating) {return document_status == status;});
    }

    vector<Document> RequestQueue::AddFindRequest(const string& raw_query) {
        return RequestQueue::AddFindRequest(raw_query, DocumentStatus::ACTUAL);
    }

    int RequestQueue::GetNoResultRequests() const {
        return null_results_;
    }
    RequestQueue::QueryResult::QueryResult(const string& raw_query, bool nul_or_one){
        query = raw_query;
        null = nul_or_one;
    }

