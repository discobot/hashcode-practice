#include <fstream>
#include <iostream>
#include <vector>
#include <random>
#include <unordered_map>
#include <unordered_set>

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

struct TCacheReq {
    size_t ServerId;
    size_t VideoId;
};

void Write(const std::string& output, const std::vector<TCacheReq>& reqs) {
    std::ofstream out(output);
    out << reqs.size() << std::endl;
    for (const auto& req : reqs) {
        out << req.ServerId << " " << req.VideoId << std::endl;
    }
    out.close();
}

std::vector<TCacheReq> RandomAssignment(const TProblem& p) {
    std::mt19937 rng;
    std::uniform_int_distribution<size_t> randomVideo(0, p.Vides.size() - 1);

    std::vector<TCacheReq> reqs;

    for (size_t i = 0; i < p.CacheCount; ++i) {
        std::unordered_set<size_t> selected;

        size_t capacity = p.CacheSize;
        while (true) {
            if (selected.size() == p.Vides.size()) {
                break;
            }

            auto videoId = randomVideo(rng);
            if (p.Vides[videoId] > capacity) {
                break;
            }

            if (selected.find(videoId) != selected.end()) {
                continue;
            }

            selected.insert(videoId);
            capacity -= p.Vides[videoId];

            reqs.emplace_back(TCacheReq{i, videoId});
        }
    }

    return reqs;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "usage: " << argv[0] << " INPUT OUTPUT" << std::endl;
        return 1;
    }

    auto p = Read(argv[1]);

    Write(argv[2], RandomAssignment(p));

    return 0;
}