#include <gtest/gtest.h>
#include <cmath>

#include "../src/nodes/planner_functions.h"

using namespace avoidance;

TEST(PlannerFunctions, generateNewHistogramEmpty) {
  // GIVEN: an empty pointcloud
	pcl::PointCloud<pcl::PointXYZ> empty_cloud;
	Histogram histogram_output = Histogram(ALPHA_RES);
	geometry_msgs::PoseStamped location;
	location.pose.position.x = 0;
	location.pose.position.y = 0;
	location.pose.position.z = 0;

  // WHEN: we build a histogram
	generateNewHistogram(histogram_output, empty_cloud, location);

  // THEN: the histogram should be all zeros
  for (int e = 0; e < GRID_LENGTH_E; e++) {
      for (int z = 0; z < GRID_LENGTH_Z; z++) {
    	  EXPECT_DOUBLE_EQ(0.0, histogram_output.get_bin(e, z));
      }
    }
}


TEST(PlannerFunctions, generateNewHistogramSpecificCells) {
  // GIVEN: a pointcloud with an object of one cell size
	Histogram histogram_output = Histogram(ALPHA_RES);
	geometry_msgs::PoseStamped location;
	location.pose.position.x = 0;
	location.pose.position.y = 0;
	location.pose.position.z = 0;
	double distance  = 1.0;

	std::vector<double> e_angle_filled = {-90, -30, 0, 20, 40, 90};
	std::vector<double> z_angle_filled = {-180, -50, 0, 59, 100, 175};
	std::vector<geometry_msgs::Point> middle_of_cell;

	for(int i = 0; i<e_angle_filled.size(); i++){
		for(int j = 0; j<z_angle_filled.size(); j++){
		   middle_of_cell.push_back(fromPolarToCartesian((int)e_angle_filled[i], (int)z_angle_filled[j], distance, location.pose.position));
		}
	}

	pcl::PointCloud<pcl::PointXYZ> cloud;
	for(int i = 0; i<middle_of_cell.size(); i++){
	  for(int j = 0; j<1; j++){
		  cloud.push_back(pcl::PointXYZ(middle_of_cell[i].x, middle_of_cell[i].y, middle_of_cell[i].z));
	  }
	}


  // WHEN: we build a histogram
    generateNewHistogram(histogram_output, cloud, location);


  // THEN: the filled cells in the histogram should be one and the others be zeros

	std::vector<int> e_index;
	std::vector<int> z_index;
	for(int i = 0; i<e_angle_filled.size(); i++){
		e_index.push_back(elevationAngletoIndex((int)e_angle_filled[i], ALPHA_RES));
		z_index.push_back(azimuthAngletoIndex((int)z_angle_filled[i], ALPHA_RES));
    }

	for (int e = 0; e < GRID_LENGTH_E; e++) {
	  for (int z = 0; z < GRID_LENGTH_Z; z++) {
		  bool e_found = std::find(e_index.begin(), e_index.end(), e) != e_index.end();
		  bool z_found = std::find(z_index.begin(), z_index.end(), z) != z_index.end();
	      if(e_found && z_found){
	   	    EXPECT_DOUBLE_EQ(1.0, histogram_output.get_bin(e, z)) << z <<", "<<e;
	      }else{
	    	EXPECT_DOUBLE_EQ(0.0, histogram_output.get_bin(e, z)) << z <<", "<<e;
	      }

	  }
	}
}

TEST(PlannerFunctions, calculateFOV) {
  // GIVEN: the horizontal and vertical Field of View, the vehicle yaw and pitc
  double h_fov = 90.0;
  double v_fov = 45.0;
  double yaw_z_greater_grid_length =
      3.14;  // z_FOV_max >= GRID_LENGTH_Z && z_FOV_min >= GRID_LENGTH_Z
  double yaw_z_max_greater_grid =
      -2.3;  // z_FOV_max >= GRID_LENGTH_Z && z_FOV_min < GRID_LENGTH_Z
  double yaw_z_min_smaller_zero = 3.9;  // z_FOV_min < 0 && z_FOV_max >= 0
  double yaw_z_smaller_zero = 5.6;      // z_FOV_max < 0 && z_FOV_min < 0
  double pitch = 0.0;

  // WHEN: we calculate the Field of View
  std::vector<int> z_FOV_idx_z_greater_grid_length;
  std::vector<int> z_FOV_idx_z_max_greater_grid;
  std::vector<int> z_FOV_idx3_z_min_smaller_zero;
  std::vector<int> z_FOV_idx_z_smaller_zero;
  int e_FOV_min;
  int e_FOV_max;

  calculateFOV(h_fov, v_fov, z_FOV_idx_z_greater_grid_length, e_FOV_min,
               e_FOV_max, yaw_z_greater_grid_length, pitch);
  calculateFOV(h_fov, v_fov, z_FOV_idx_z_max_greater_grid, e_FOV_min, e_FOV_max,
               yaw_z_max_greater_grid, pitch);
  calculateFOV(h_fov, v_fov, z_FOV_idx3_z_min_smaller_zero, e_FOV_min,
               e_FOV_max, yaw_z_min_smaller_zero, pitch);
  calculateFOV(h_fov, v_fov, z_FOV_idx_z_smaller_zero, e_FOV_min, e_FOV_max,
               yaw_z_smaller_zero, pitch);

  // THEN:
  std::vector<int> output_z_greater_grid_length = {
      7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21};
  std::vector<int> output_z_max_greater_grid = {0, 1, 2,  3,  4,  5,  6, 7,
                                                8, 9, 10, 11, 12, 58, 59};
  std::vector<int> output_z_min_smaller_zero = {0, 1, 2,  3,  4,  5,  6, 7,
                                                8, 9, 10, 11, 12, 13, 59};
  std::vector<int> output_z_smaller_zero = {43, 44, 45, 46, 47, 48, 49, 50,
                                            51, 52, 53, 54, 55, 56, 57, 58};

  EXPECT_EQ(18, e_FOV_max);
  EXPECT_EQ(10, e_FOV_min);
  for (size_t i = 0; i < z_FOV_idx_z_greater_grid_length.size(); i++) {
    EXPECT_EQ(output_z_greater_grid_length.at(i),
              z_FOV_idx_z_greater_grid_length.at(i));
  }

  for (size_t i = 0; i < z_FOV_idx_z_max_greater_grid.size(); i++) {
    EXPECT_EQ(output_z_max_greater_grid.at(i),
              z_FOV_idx_z_max_greater_grid.at(i));
  }

  for (size_t i = 0; i < z_FOV_idx3_z_min_smaller_zero.size(); i++) {
    EXPECT_EQ(output_z_min_smaller_zero.at(i),
              z_FOV_idx3_z_min_smaller_zero.at(i));
  }

  for (size_t i = 0; i < z_FOV_idx_z_smaller_zero.size(); i++) {
    EXPECT_EQ(output_z_smaller_zero.at(i), z_FOV_idx_z_smaller_zero.at(i));
  }
}