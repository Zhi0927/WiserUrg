#ifndef ZHI_CLUSTER2D_H_
#define ZHI_CLUSTER2D_H_

#include <chrono>
#include <string>
#include "KdTree2D.hpp"


class ClusterPts {
public:
    ClusterPts(int nPts, float cTol, int minSize, int maxSize);
    ~ClusterPts();

    //void clusterHelper(int ind, PtCdtr<PointT> cloud, std::vector<int>& cluster, KdTree* tree);
    void clusterHelper(int idx, const std::vector<std::vector<float>>& points, std::vector<int>& cluster, KdTree2D* KdTree);
    std::vector<std::vector<int>> EuclidCluster(const std::vector<std::vector<float>>& points, KdTree2D* KdTree);

private:
    int                             m_pointsNum;
    float                           m_distanceTol;
    int                             m_minClusterSize;
    int                             m_maxClusterSize;
    std::vector<bool>               m_processed;
    std::vector<std::vector<int>>   m_clusters;
};
#endif
