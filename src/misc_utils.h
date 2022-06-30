#pragma once

#include <unordered_set>

// Set union

std::unordered_set<std::size_t> operator + (const std::unordered_set<std::size_t>& a,
                                            const std::unordered_set<std::size_t>& b);

std::unordered_set<std::size_t> operator + (std::unordered_set<std::size_t>&& a,
                                            const std::unordered_set<std::size_t>& b);

std::unordered_set<std::size_t>& operator += (std::unordered_set<std::size_t>& a,
                                            const std::unordered_set<std::size_t>& b);

// Set difference

std::unordered_set<std::size_t> operator - (const std::unordered_set<std::size_t>& a,
                                            const std::unordered_set<std::size_t>& b);

std::unordered_set<std::size_t> operator - (std::unordered_set<std::size_t>&& a,
                                            const std::unordered_set<std::size_t>& b);

// Interesection

std::unordered_set<std::size_t> operator & (const std::unordered_set<std::size_t>& a,
                                            const std::unordered_set<std::size_t>& b);


std::unordered_set<std::size_t> operator & (std::unordered_set<std::size_t>&& a,
                                            const std::unordered_set<std::size_t>& b);

std::unordered_set<std::size_t>& operator &= (std::unordered_set<std::size_t>& a,
                                            const std::unordered_set<std::size_t>& b);

// Symmetric difference

std::unordered_set<std::size_t> operator ^ (const std::unordered_set<std::size_t>& a,
                                            const std::unordered_set<std::size_t>& b);
