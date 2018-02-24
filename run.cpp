#include <fstream>
#include <iostream>
#include <vector>

struct TCacheConn {
    size_t ServerId;
    size_t Latency;
};

struct TEndpoint {
    size_t Latency;
    std::vector<TCacheConn> Caches;
};

struct TRequest {
    size_t VideoId;
    size_t EndpointId;
    size_t RequestCount;
};

struct TProblem {
    std::vector<size_t> Vides;
    std::vector<TEndpoint> Endpoints;
    std::vector<TRequest> Requests;

    size_t CacheCount;
    size_t CacheSize;
};

TProblem Read(const std::string& file) {
    std::ifstream in(file);
    in.exceptions(std::ios_base::failbit | std::ios_base::badbit);

    TProblem p;

    int V, E, R;
    in >> V >> E >> p.CacheCount >> p.CacheSize;

    for (size_t i = 0; i < V; ++i) {
        in >> p.Vides.emplace_back();
    }

    for (size_t i = 0; i < E; ++i) {
        auto& e = p.Endpoints.emplace_back();
        in >> e.Latency;
        size_t N;
        in >> N;
        for (size_t j = 0; j < N; ++j) {
            auto& c = e.Caches.emplace_back();
            in >> c.ServerId >> c.Latency;
        }
    }

    for (size_t i = 0; i < R; ++i) {
        auto& r = p.Requests.emplace_back();
        in >> r.VideoId >> r.EndpointId >> r.RequestCount;
    }

    return p;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "usage: " << argv[0] << " INPUT OUTPUT" << std::endl;
        return 1;
    }

    auto p = Read(argv[0]);


    return 0;
}