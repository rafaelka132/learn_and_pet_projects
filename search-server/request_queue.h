#pragma once

#include "search_server.h"
class RequestQueue {
public:
    explicit RequestQueue(const SearchServer& search_server)
        : search_server_(search_server) {
        // Инициализация завершена в списке инициализации
    }

    template <typename DocumentPredicate>
    vector<Document> AddFindRequest(const string& raw_query, DocumentPredicate document_predicate) {
        vector<Document> find_docs = search_server_.FindTopDocuments(raw_query, document_predicate);
        return ProcessRequest(raw_query, find_docs);
    }

    vector<Document> AddFindRequest(const string& raw_query, DocumentStatus status) {
        vector<Document> find_docs = search_server_.FindTopDocuments(raw_query, status);
        return ProcessRequest(raw_query, find_docs);
    }

    vector<Document> AddFindRequest(const string& raw_query) {
        vector<Document> find_docs = search_server_.FindTopDocuments(raw_query);
        return ProcessRequest(raw_query, find_docs);
    }

    int GetNoResultRequests() const {
        int result = 0;
        // Используем std::accumulate для подсчета запросов без результатов
        result = accumulate(requests_.begin(), requests_.end(), 0, [](int sum, const QueryResult& query) {
            return sum + (query.is_empty ? 1 : 0);
            });
        return result;
    }

private:
    struct QueryResult {
        vector<Document> documents_; // Список найденных документов
        bool is_empty;               // Флаг, указывающий, были ли найдены документы
        int time;                   // Время запроса
    };;

    deque<QueryResult> requests_; // Очередь запросов
    const static int min_in_day_ = 1440; // Минимальное количество минут в дне
    const SearchServer& search_server_; // Ссылка на объект SearchServer
    int time_now = 0;
    // Вспомогательный метод для обработки запросов

    vector<Document> ProcessRequest(const string& raw_query, vector<Document> found_documents) {
        // Удаляем старые запросы из очереди
  
        while (!requests_.empty() && time_now - requests_.front().time >= min_in_day_) {
            requests_.pop_front();
        }
        

        // Создаем структуру для хранения результатов запроса
        QueryResult query_result;
        query_result.documents_ = found_documents;
        query_result.is_empty = found_documents.empty();
        query_result.time = time_now;

        // Добавляем результат запроса в очередь
        requests_.push_back(query_result);

        // Уменьшаем оставшееся время на 1
        ++time_now;
        return found_documents;
    }
};
