import sys
from collections import defaultdict

def read_tuple(f):
    return map(int, f.strip().split())


def score(f, sol):
    V, E, R, C, X = read_tuple(f)

    video_sizes = {}
    for i in range(V):
        video_sizes[i] = int(f.readline().strip())

    connections = {}
    main_latency = {}

    for endpoint in range(E):
        main_latency[endpoint], servers_count = read_tuple(f)

        connections[endpoint] = defaultdict(lambda : main_latency[endpoint])
        
        for _ in range(servers_count):
            server, latency = read_tuple(f)
            connections[i][server] = latency



    video_servers = defaultdict(list)
    for i in range(int(sol.readline().strip())):
        server, *videos = read_tuple()

        assert(sum(video_sizes[v] for v in videos) <= X, "tooo muuuchh")
        for v in videos:
            video_servers[video].append(server)

    time_saved = 0
    total_count = 0

    requests = defaultdict(list)    
    for r in range(R):
        video, endpoint, count = read_tuple(f)
        requests[video].append((endpoint, count))

        best_latency = main_latency[endpoint]

        for server in video_servers[video]:
            best_latency = min(best_latency, connections[endpoint][server])

        time_saved += (main_latency[endpoint] - best_latency) * count
        total_count += count

    return int(time_saved / total_count * 1000)




if __name__ == "__main__":
    print(score(open(sys.argv[1]), open(sys.argv[2])))




