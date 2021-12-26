#pragma once

#include <drag/detail/subgraph.hpp>
#include <drag/detail/layering.hpp>


/**
 * Compute the smallest possible total edge length in a hierarchy
 * using a bruteforce approach.
 * Finishes in reasonable time only for small graphs (about 15 nodes).
 * 
 * In this case the length of a single edge is the number of "empty spaces"
 * between layers it spans in the hierarchy.
 * E.g. for an edge (u, v), its length is rank(v) - rank(u).
 * 
 * @param g the input graph
 * 
 * @return the smallest possible total edge length among all layerings
 */
size_t bruteforce_layering_total_length(const drag::graph& g);
