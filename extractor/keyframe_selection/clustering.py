import numpy as np


class IterativeBestMerge1D:

    def __init__(self, distances: np.ndarray, method="max"):
        assert method == "max" or method == "average", "only 'max' and 'average' method supported"
        assert len(distances.shape) == 2, "distances must be 2 dimensional [n, n] numpy array"

        self._inp_len = len(distances)
        self._distances = np.empty([self._inp_len * 2 - 1, self._inp_len * 2 - 1], np.float64)
        self._distances[:] = np.nan
        self._distances[:self._inp_len, :self._inp_len] = distances

        self._cluster_idx = np.empty([self._inp_len * 2 - 1, 2], np.int32)
        self._cluster_idx[:self._inp_len, 0] = np.arange(self._inp_len, dtype=np.int32)
        self._cluster_idx[:self._inp_len, 1] = np.arange(self._inp_len, dtype=np.int32) + 1
        self._clusters = np.arange(self._inp_len, dtype=np.int32).tolist()

        if method == "max":
            def _max(cluster1, cluster2):
                from1, to1 = self._cluster_idx[cluster1]
                from2, to2 = self._cluster_idx[cluster2]
                return np.max(self._distances[from1:to1, from2:to2])
            self._method = _max
        elif method == "average":
            def _average(cluster1, cluster2):
                from1, to1 = self._cluster_idx[cluster1]
                from2, to2 = self._cluster_idx[cluster2]
                return np.mean(self._distances[from1:to1, from2:to2])
            self._method = _average

        self._Z = np.empty([self._inp_len - 1, 4], np.float64)

        self._compute()

    def _merge(self, cluster1, cluster2, iteration, distance):
        curr = self._inp_len + iteration
        self._cluster_idx[curr, 0] = self._cluster_idx[cluster1, 0]
        self._cluster_idx[curr, 1] = self._cluster_idx[cluster2, 1]

        size = self._cluster_idx[curr, 1] - self._cluster_idx[curr, 0]
        self._Z[iteration] = np.array([cluster1, cluster2, distance, size])

        idx = self._clusters.index(cluster1)
        # assert self._clusters[idx + 1] == cluster2
        self._clusters = self._clusters[:idx] + [curr] + self._clusters[idx + 2:]

    def _compute(self):
        self.no_steps, iteration = 0, 0

        while iteration < self._inp_len - 1:
            min_dist = np.inf
            merge_clusters = []

            for idx, curr in enumerate(self._clusters):
                if idx > 0:
                    prev = self._clusters[idx - 1]
                    dist = self._distances[prev, curr]
                    if np.isnan(dist):
                        dist = self._method(prev, curr)
                        self._distances[prev, curr] = dist

                    if dist < min_dist:
                        min_dist = dist
                        merge_clusters = [prev, curr]

                if idx < len(self._clusters) - 1:
                    next = self._clusters[idx + 1]
                    dist = self._distances[curr, next]
                    if np.isnan(dist):
                        dist = self._method(curr, next)
                        self._distances[curr, next] = dist

                    if dist < min_dist:
                        min_dist = dist
                        merge_clusters = [curr, next]

                self.no_steps += 1

            self._merge(*merge_clusters, iteration, min_dist)
            iteration += 1

    @property
    def linkage(self):
        return self._Z


def _get_max_dist_for_each_cluster(linkage: np.ndarray):
    """
    Get the maximum inconsistency coefficient for each non-singleton cluster.
    Adapted from https://github.com/scipy/scipy/blob/v1.3.0/scipy/cluster/_hierarchy.pyx.
    """
    n = len(linkage) + 1
    curr_node = np.zeros([n - 1], np.int32)
    visited = np.zeros([n - 1], np.bool)
    md = np.zeros([n - 1], np.float64)

    k = 0
    curr_node[0] = 2 * n - 2
    while k >= 0:
        root = curr_node[k] - n
        i_lc = int(linkage[root, 0])
        i_rc = int(linkage[root, 1])

        if i_lc >= n and not visited[i_lc - n]:
            visited[i_lc - n] = True
            k += 1
            curr_node[k] = i_lc
            continue

        if i_rc >= n and not visited[i_rc - n]:
            visited[i_rc - n] = True
            k += 1
            curr_node[k] = i_rc
            continue

        max_dist = linkage[root, 2]
        if i_lc >= n:
            max_l = md[i_lc - n]
            if max_l > max_dist:
                max_dist = max_l
        if i_rc >= n:
            max_r = md[i_rc - n]
            if max_r > max_dist:
                max_dist = max_r
        md[root] = max_dist

        k -= 1
    return md


def _form_clusters(linkage: np.ndarray, md: np.ndarray, cluster_func):
    """
    Form clusters.
    cluster_func(distance, number_of_nodes) -> True (create cluster) or False (do not create cluster)
    Adapted from https://github.com/scipy/scipy/blob/v1.3.0/scipy/cluster/_hierarchy.pyx.
    """
    n_cluster = 0
    cluster_leader = -1

    n = len(linkage) + 1
    curr_node = np.zeros([n - 1], np.int32)
    visited = np.zeros([n - 1], np.bool)
    cluster_assignments = np.zeros([n], np.int32)

    k = 0
    curr_node[0] = 2 * n - 2
    while k >= 0:
        root = curr_node[k] - n
        i_lc = int(linkage[root, 0])
        i_rc = int(linkage[root, 1])

        if cluster_leader == -1 and cluster_func(md[root], int(linkage[i_lc - n, 3]) if i_lc >= n else 1,
                                                 int(linkage[i_rc - n, 3]) if i_rc >= n else 1):  # found a cluster
            cluster_leader = root
            n_cluster += 1

        if i_lc >= n and not visited[i_lc - n]:
            visited[i_lc - n] = True
            k += 1
            curr_node[k] = i_lc
            continue

        if i_rc >= n and not visited[i_rc - n]:
            visited[i_rc - n] = True
            k += 1
            curr_node[k] = i_rc
            continue

        if i_lc < n:
            if cluster_leader == -1:  # singleton cluster
                n_cluster += 1
            cluster_assignments[i_lc] = n_cluster

        if i_rc < n:
            if cluster_leader == -1:  # singleton cluster
                n_cluster += 1
            cluster_assignments[i_rc] = n_cluster

        if cluster_leader == root:  # back to the leader
            cluster_leader = -1
        k -= 1

    return cluster_assignments


def form_clusters(linkage: np.ndarray, cluster_func):
    md = _get_max_dist_for_each_cluster(linkage)
    return _form_clusters(linkage, md, cluster_func)
