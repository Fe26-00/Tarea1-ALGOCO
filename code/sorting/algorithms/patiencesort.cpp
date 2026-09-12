/*
Algoritmo basado en:
https://www.baeldung.com/cs/patience-sort-algorithm
(altamente modificado)
*/
#include <vector>
#include <algorithm>
#include <queue>
using namespace std;

vector<int> patienceSorting(vector<int> arr) {
    vector<vector<int>> piles;
    vector<int> tops; // tope de cada montón, siempre queda ordenado ascendentemente

    for (int x : arr) {
        // buscar el montón más a la izquierda cuyo tope sea > x
        auto it = upper_bound(tops.begin(), tops.end(), x);
        if (it == tops.end()) {
            // ningún montón sirve: crear uno nuevo
            piles.push_back({x});
            tops.push_back(x);
        } else {
            int idx = it - tops.begin();
            piles[idx].push_back(x);
            tops[idx] = x;
        }
    }

    // Fusionar montones con un min-heap: (valor, indice_del_monton)
    using P = pair<int, int>;
    priority_queue<P, vector<P>, greater<P>> pq;
    for (int i = 0; i < (int)piles.size(); i++) {
        pq.push({piles[i].back(), i});
    }

    vector<int> ans;
    ans.reserve(arr.size());

    while (!pq.empty()) {
        auto [val, idx] = pq.top();
        pq.pop();
        ans.push_back(val);
        piles[idx].pop_back();
        if (!piles[idx].empty()) {
            pq.push({piles[idx].back(), idx});
        }
    }

    return ans;
}