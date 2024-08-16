#include <iostream>
#include <limits.h>
#include <networkit/graph/Graph.hpp>
#include <networkit/io/METISGraphReader.hpp>
#include <networkit/io/METISGraphWriter.hpp>

std::vector<NetworKit::node> kHopNeighbours(NetworKit::Graph g, NetworKit::node u, int k) {
    std::queue<std::pair<NetworKit::node, int> > queue;

    std::vector<NetworKit::node> neighbours;
    std::vector<bool> visited(g.numberOfNodes(), false);
    queue.emplace(u, 0);
    visited[u] = true;

    while (!queue.empty()) {
        auto [v, dist] = queue.front();
        queue.pop();
        g.forNeighborsOf(v, [&](NetworKit::node x) {
            if (visited[x]) return;
            visited[x] = true;
            neighbours.push_back(x);
            if (dist < k)
                queue.emplace(x, dist + 1);
        });
    }

    return neighbours;
}

int main(int argc, char **args) {
    if (argc != 4 && argc != 5) {
        std::cout << "Needs 3 or 4 arguments: k input_path output_path [log2_max_edges]\n";
        return -1;
    }
    std::cout << "kHop Graph Generator\n";
    int k = atoi(args[1]);
    char *input_path = args[2];
    char *output_path = args[3];
    NetworKit::edgeid max_edges = argc == 5 ? 1ul << atoi(args[4]) : ULONG_MAX;

    std::cout << "Max Edges: " << max_edges << "\n";

    std::cout << "Reading input graph...\n";
    NetworKit::METISGraphReader reader;
    NetworKit::Graph input_graph = reader.read(input_path);

    std::cout << "Generating...\n";
    auto output_graph = NetworKit::Graph(input_graph.numberOfNodes());
    if (input_graph.isDirected()) {
        input_graph.parallelForNodes([&](NetworKit::node u) {
            for (auto v: kHopNeighbours(input_graph, u, k)) {
                output_graph.addEdge(u, v, NetworKit::defaultEdgeWeight, true);
            }
            if(output_graph.numberOfEdges() > max_edges) throw std::length_error("too many edges");
        });
    } else {
        input_graph.parallelForNodes([&](NetworKit::node u) {
            for (NetworKit::node v: kHopNeighbours(input_graph, u, k)) {
                if (u < v)
                    output_graph.addEdge(u, v);
            }
            if(output_graph.numberOfEdges() > max_edges) throw std::length_error("too many edges");
        });
    }

    std::cout << "Writing...\n";
    NetworKit::METISGraphWriter writer;
    writer.write(output_graph, output_path);
}
