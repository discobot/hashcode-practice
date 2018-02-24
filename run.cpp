#include <fstream>
#include <iostream>
#include <vector>
#include <random>
#include <queue>
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

struct TEndpointCache {
    size_t EndpointId;
    size_t ServerId;

    const bool operator==(const TEndpointCache& ec) const {
        return EndpointId == ec.EndpointId && ServerId == ec.ServerId;
    }
};

struct TEndpointVideo {
    size_t EndpointId;
    size_t VideoId;

    const bool operator==(const TEndpointVideo& ev) const {
        return EndpointId == ev.EndpointId && VideoId == ev.VideoId;
    }
};

struct TCacheReq {
    size_t ServerId;
    size_t VideoId;

    const bool operator==(const TCacheReq& r) const {
        return ServerId == r.ServerId && VideoId == r.VideoId;
    }
};


namespace std
{
    template<> struct hash<TEndpointVideo>
    {
        size_t operator()(TEndpointVideo const& r) const noexcept
        {
            auto h1 = std::hash<size_t>{}(r.EndpointId);
            auto h2 = std::hash<size_t>{}(r.VideoId);
            return h1 ^ (h2 << 3);
        }
    };

    template<> struct hash<TEndpointCache>
    {
        size_t operator()(TEndpointCache const& r) const noexcept
        {
            auto h1 = std::hash<size_t>{}(r.EndpointId);
            auto h2 = std::hash<size_t>{}(r.ServerId);
            return h1 ^ (h2 << 3);
        }
    };

    template<> struct hash<TCacheReq>
    {
        size_t operator()(TCacheReq const& r) const noexcept
        {
            auto h1 = std::hash<size_t>{}(r.ServerId);
            auto h2 = std::hash<size_t>{}(r.VideoId);
            return h1 ^ (h2 << 3);
        }
    };
}

struct TProblem {
    std::vector<size_t> Vides;
    std::vector<TEndpoint> Endpoints;
    std::vector<TRequest> Requests;
    std::vector<std::vector<size_t>> VideoEndpoints;
    std::vector<size_t> Capacities;
    std::unordered_map<TEndpointCache, size_t> Latencies;
    std::unordered_map<TEndpointVideo, size_t> RequestsCount;

    size_t CacheCount;
    size_t CacheSize;
};

TProblem Read(const std::string& file) {
    std::ifstream in(file);
    in.exceptions(std::ios_base::failbit | std::ios_base::badbit);

    TProblem p;

    int V, E, R;
    in >> V >> E >> R >> p.CacheCount >> p.CacheSize;

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
            p.Latencies[{i, c.ServerId}] = c.Latency;
        }
    }

    p.VideoEndpoints.resize(V);
    for (size_t i = 0; i < R; ++i) {
        auto& r = p.Requests.emplace_back();
        in >> r.VideoId >> r.EndpointId >> r.RequestCount;
        p.VideoEndpoints[r.VideoId].push_back(r.EndpointId);
        p.RequestsCount[{r.EndpointId, r.VideoId}] = r.RequestCount;
    }

    p.Capacities.resize(p.CacheCount);
    for (size_t i = 0; i < p.CacheCount; ++i) {
        p.Capacities[i] = p.CacheSize;
    }
    std::cerr << "Data read\n";
    return p;
}

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

size_t CalcPriority(
        TProblem& p,
        std::unordered_map<TEndpointVideo, size_t>& serveTime,
        TCacheReq& candidate
    ) {
    size_t savedTime = 0;
    for (size_t e : p.VideoEndpoints[candidate.VideoId]) {
        size_t currentServeTime = serveTime[{e, candidate.VideoId}];
        auto foundLatency = p.Latencies.find({e, candidate.ServerId});
        auto foundRequestCount = p.RequestsCount.find({e, candidate.VideoId});
        if (foundLatency != p.Latencies.end() &&
            foundLatency->second < currentServeTime &&
            foundRequestCount != p.RequestsCount.end() &&
            p.Capacities[candidate.ServerId] >= p.Vides[candidate.VideoId]) {
            savedTime += foundRequestCount->second * (currentServeTime - foundLatency->second);
        }
    }
    return savedTime;
}

typedef std::pair<TCacheReq, size_t> QueueEntry;

std::vector<TCacheReq> PriorityQueueAssignment(TProblem& p) {
    std::vector<TCacheReq> result;
    std::unordered_map<TEndpointVideo, size_t> serveTime;
    std::unordered_set<TCacheReq> addedCacheRequests;
    for (size_t e = 0; e < p.Endpoints.size(); ++e) {
        for (size_t v = 0; v < p.Vides.size(); ++v) {
            serveTime[{e, v}] = p.Endpoints[e].Latency;
        }
    }
    auto cmp = [&](auto& lhs, auto& rhs) { return lhs.second < rhs.second; };
    std::priority_queue<QueueEntry, std::vector<QueueEntry>, decltype(cmp)> queue(cmp);
    for (auto& request : p.Requests) {
        for (auto& cacheConn : p.Endpoints[request.EndpointId].Caches) {
            TCacheReq cacheRequest{cacheConn.ServerId, request.VideoId};
            if (addedCacheRequests.count(cacheRequest)) {
                continue;
            }
            addedCacheRequests.insert(cacheRequest);
            queue.push({cacheRequest, CalcPriority(p, serveTime, cacheRequest)});
        }
    }
    std::cerr << "Starting\n" << std::flush;
    while (!queue.empty()) {
        std::cerr << "\rQueue size: " << queue.size() << std::flush;
        auto entry = queue.top();
        queue.pop();
        auto cacheRequest = entry.first;
        if (p.Capacities[cacheRequest.ServerId] < p.Vides[cacheRequest.VideoId]) {
            continue;
        }
        size_t priority = entry.second;
        size_t newPriority = CalcPriority(p, serveTime, cacheRequest);
        if (entry.second != newPriority) {
            queue.push({cacheRequest, newPriority});
            continue;
        }
        if (p.Capacities[cacheRequest.ServerId] < p.Vides[cacheRequest.VideoId]) {
            continue;
        }
        result.push_back(cacheRequest);
        p.Capacities[cacheRequest.ServerId] -= p.Vides[cacheRequest.VideoId];
        for (size_t e = 0; e < p.Endpoints.size(); ++e) {
            auto foundLatency = p.Latencies.find({e, cacheRequest.ServerId});
            if (foundLatency == p.Latencies.end()) {
                continue;
            }
            serveTime[{e, cacheRequest.VideoId}] =
                std::min(serveTime[{e, cacheRequest.VideoId}], foundLatency->second);
        }
    }
    std::cerr << "\n";
    return result;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "usage: " << argv[0] << " INPUT OUTPUT" << std::endl;
        return 1;
    }

    auto p = Read(argv[1]);

    Write(argv[2], PriorityQueueAssignment(p));

    return 0;
}
