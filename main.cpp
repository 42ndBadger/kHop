#include <iostream>
#include <bits/parse_numbers.h>
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
    if (argc != 4) {
        std::cout << "Needs 3 arguments: k input_path output_path\n";
        return -1;
    }
    std::cout << "kHop Graph Generator\n";
    int k = atoi(args[1]);
    char *input_path = args[2];
    char *output_path = args[3];

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
        });
    } else {
        input_graph.parallelForNodes([&](NetworKit::node u) {
            for (NetworKit::node v: kHopNeighbours(input_graph, u, k)) {
                if (u < v)
                    output_graph.addEdge(u, v);
            }
        });
    }

    std::cout << "Writing...\n";
    NetworKit::METISGraphWriter writer;
    writer.write(output_graph, output_path);
}
