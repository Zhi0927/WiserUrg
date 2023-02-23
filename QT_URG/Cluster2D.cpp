#include "Cluster2D.h"


ClusterPts::ClusterPts(int _pointNum, float cTol, int minSize, int maxSize):
    m_pointsNum(_pointNum),
    m_distanceTol(cTol),
    m_minClusterSize(minSize),
    m_maxClusterSize(maxSize)
{
    m_processed.assign(m_pointsNum, false);
}

ClusterPts::~ClusterPts() {}


void ClusterPts::clusterHelper(int idx, const std::vector<std::vector<float>>& points, std::vector<int>& cluster, KdTree2D* KdTree) {
    m_processed[idx] = true;
    cluster.push_back(idx);

    std::vector<int> nearestPoint = KdTree->search(points[idx], m_distanceTol);
    for (int nearestId : nearestPoint) {
        if (!m_processed[nearestId]) {
            clusterHelper(nearestId, points, cluster, KdTree);
        }
    }
}


//std::vector<std::vector<int>> ClusterPts::EuclidCluster(const std::vector<std::vector<float>>& points) {
//    KdTree2D* KdTree = new KdTree2D;
//    for (int ind = 0; ind < m_pointsNum; ind++) {
//        KdTree->insert(cloud->points[ind], ind);
//    }
//    for (int ind = 0; ind < m_pointsNum; ind++) {
//        if (m_processed[ind]) {
//            ind++;
//            continue;
//        }
//        std::vector<int> cluster_ind;
//        PtCdtr<PointT> cloudCluster(new pcl::PointCloud<PointT>);
//        clusterHelper(ind, cloud, cluster_ind, KdTree);
//
//        int cluster_size = cluster_ind.size();
//        if (cluster_size >= m_minClusterSize && cluster_size <= m_maxClusterSize) {
//            for (int i = 0; i < cluster_size; i++) {
//                cloudCluster->points.push_back(cloud->points[cluster_ind[i]]);
//            }
//            cloudCluster->width = cloudCluster->points.size();
//            cloudCluster->height = 1;
//            m_clusters.push_back(cloudCluster);
//        }
//    }
//    return m_clusters;
//}

std::vector<std::vector<int>> ClusterPts::EuclidCluster(const std::vector<std::vector<float>>& points, KdTree2D* KdTree) {

    // TODO: Fill out this function to return list of indices for each cluster

    std::vector<std::vector<int>> clusters;
    std::vector<bool> processed(points.size(), false);

    int i = 0;
    while (i < points.size()) {
        if (processed[i]) {
            i++;
            continue;
        }
        std::vector<int> cluster;
        clusterHelper(i, points, cluster, KdTree);
        clusters.push_back(cluster);
        i++;
    }

    return clusters;
}