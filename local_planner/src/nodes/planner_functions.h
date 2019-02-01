#ifndef LOCAL_PLANNER_FUNCTIONS_H
#define LOCAL_PLANNER_FUNCTIONS_H

#include "box.h"
#include "candidate_direction.h"
#include "common.h"
#include "cost_parameters.h"
#include "histogram.h"

#include <Eigen/Dense>

#include <geometry_msgs/Point.h>
#include <geometry_msgs/PoseStamped.h>

#include <pcl/point_cloud.h>
#include <pcl/point_types.h>

#include <nav_msgs/GridCells.h>
#include <nav_msgs/Path.h>

#include <queue>
#include <vector>

namespace avoidance {

void filterPointCloud(
    pcl::PointCloud<pcl::PointXYZ>& cropped_cloud,
    Eigen::Vector3f& closest_point, float& distance_to_closest_point,
    int& counter_backoff,
    const std::vector<pcl::PointCloud<pcl::PointXYZ>>& complete_cloud,
    int min_cloud_size, float min_dist_backoff, Box histogram_box,
    const Eigen::Vector3f& position, float min_realsense_dist);
void calculateFOV(float h_FOV, float v_FOV, std::vector<int>& z_FOV_idx,
                      int& e_FOV_min, int& e_FOV_max, float yaw, float pitch);
void propagateHistogram(
    Histogram& polar_histogram_est,
    const pcl::PointCloud<pcl::PointXYZ>& reprojected_points,
    const std::vector<int>& reprojected_points_age,
    const geometry_msgs::PoseStamped& position);
void generateNewHistogram(Histogram& polar_histogram,
                          const pcl::PointCloud<pcl::PointXYZ>& cropped_cloud,
                          const Eigen::Vector3f& position);
void combinedHistogram(bool& hist_empty, Histogram& new_hist,
                       const Histogram& propagated_hist,
                       bool waypoint_outside_FOV,
                       const std::vector<int>& z_FOV_idx, int e_FOV_min,
                       int e_FOV_max);
void compressHistogramElevation(Histogram& new_hist,
                                const Histogram& input_hist);
void getCostMatrix(const Histogram& histogram, const Eigen::Vector3f& goal,
                   const Eigen::Vector3f& position,
                   const Eigen::Vector3f& last_sent_waypoint,
                   costParameters cost_params, bool only_yawed,
                   Eigen::MatrixXf& cost_matrix);
void getBestCandidatesFromCostMatrix(
    const Eigen::MatrixXf& matrix, unsigned int number_of_candidates,
    std::vector<candidateDirection>& candidate_vector);
float costFunction(float e_angle, float z_angle, float obstacle_distance,
                   const Eigen::Vector3f& goal, const Eigen::Vector3f& position,
                   const Eigen::Vector3f& last_sent_waypoint,
                   costParameters cost_params, bool only_yawed);
void smoothPolarMatrix(Eigen::MatrixXf& matrix, unsigned int smoothing_radius);
void padPolarMatrix(const Eigen::MatrixXf& matrix, unsigned int n_lines_padding,
                    Eigen::MatrixXf& matrix_padded);
void printHistogram(Histogram& histogram);
bool getDirectionFromTree(
    PolarPoint& p_pol,
    const std::vector<geometry_msgs::Point>& path_node_positions,
    const Eigen::Vector3f& position);
}
#endif  // LOCAL_PLANNER_FUNCTIONS_H
