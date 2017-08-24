#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <sstream> //Handling input
#include <stdlib.h> //Random numbers
#include <math.h> //Power function
#include <iomanip> //Setprecision for printing results

#define NUM_ITERATIONS 100 //Number of random iterations to perform
#define DATA_MAX 15 //Max value of data for random cluster placement 
#define DATA_MIN 0.0 //Min value of data for random cluster placement


/********************* NOTES *************************

- If 2 coords are same distance from cluster it will 
  be classified to the lower valued cluster.

- Cluster initialization is random within defined
  data range

*****************************************************/


//Classify's data to it's nearest cluster, the cluster it belongs to is stored in first element of coordinate vector
void classifyData(std::vector<std::vector<double>> &data, std::vector<std::vector<double>> &clusterCoordinates)
{
	for (int i=0; i<data.size(); i++)
	{
		double minDistance = 2*(DATA_MAX-DATA_MIN)*(DATA_MAX-DATA_MIN);
		double minCluster = -1;
		for (int j=0; j<clusterCoordinates.size(); j++)
		{
			double distance = 0;
			for (int w=0; w<clusterCoordinates[j].size(); w++)
			{
				distance += pow(clusterCoordinates[j][w] - data[i][w+1], 2);
			}

			if (distance < minDistance)
			{
				minDistance = distance;
				minCluster = j;
			}
		}
		data[i][0] = minCluster;
	}
}

//Updates cluster location to new average, returns true if we have converged
bool updateClusterLoc(std::vector<std::vector<double>> &data, std::vector<std::vector<double>> &clusterCoordinates)
{
	std::vector<std::vector<double>> prevClusterCoordinates = clusterCoordinates;n
	for (int i=0; i<clusterCoordinates.size(); i++) //Iterate through clusters
	{
		for (int j=0; j<clusterCoordinates[i].size(); j++) //Iterate through coordinates
		{
			//Sum total value for this coordinate
			double sum = 0;
			double count = 0;
			for (int w=0; w<data.size(); w++)
			{
				if (data[w][0] == i) //Check if belongs to cluster
				{
					sum += data[w][j+1];
					count++;
				}
			}
			if (count != 0) clusterCoordinates[i][j] = sum/count;
		}
	}
	return prevClusterCoordinates == clusterCoordinates;
}

//Computes total sum of distances from coordinates to their corresponding cluster
double computeClusterDistance(std::vector<std::vector<double>> &data, std::vector<std::vector<double>> &clusterCoordinates)
{
	double distance = 0;
	for (int i=0; i<data.size(); i++)
	{
		for (int j=0; j<clusterCoordinates.size(); j++)
		{
			for (int w=0; w<clusterCoordinates[j].size(); w++)
			{
				if (data[i][0] == j) //Check if belongs to cluster
				{
					distance += pow(clusterCoordinates[j][w] - data[i][w+1], 2);
				}
			}
		}
	}
	return distance;
}

//Prints cluster coordinates and the data points that belong to them
void printResults(std::vector<std::vector<double>> &data, std::vector<std::vector<double>> &clusterCoordinates)
{
	for (int i=0; i<clusterCoordinates.size(); i++)
	{
		std::cout << "Cluster coordinate " << i << ": ";
		for (int j=0; j<clusterCoordinates[i].size(); j++)
		{
			std::cout << clusterCoordinates[i][j] << " ";
		}
		std::cout << std::endl;
	}
	std::cout << std::fixed << std::setprecision(2);
	for (int i=0; i<data.size(); i++)
	{
		std::cout << "Data " << i << " in cluster " << data[i][0] << ", coordinates: ";
		for (int j=1; j<data[i].size(); j++)
		{
			std::cout << data[i][j] << " ";
		}
		std::cout << std::endl;
	}
}

//Outputs data in correct format to image file
void compressImage	(std::vector<std::vector<double>> &data, 
					std::vector<std::vector<double>> &clusterCoordinates, 
					std::string fileName,
					std::vector<std::string> &imageHeader)
{
	std::string outputFile = fileName + "-compressed.ppm";
	std::ofstream output(outputFile);
	for (int i=0; i<imageHeader.size(); i++)
	{
		output << imageHeader[i] << std::endl;
	}
	for (int i=0; i<data.size(); i+=5)
	{
		for (int w=0; w<5; w++)
		{
			if (i+w*3 < data.size())
			{
				for (int j=0; j<clusterCoordinates[0].size(); j++)
				{
					output << (int)clusterCoordinates[data[i+w*3][0]][j] << " ";
				}
			}
		}
		output << std::endl;
	}
}

//Runs kmeans clustering on data vector, if ppm outputs compressed file
void kmeans	(std::vector<std::vector<double>> &data, 
			int numClusters, 
			bool ppm, 
			std::string fileName,
			std::vector<std::string> &imageHeader)
{
	std::vector<std::vector<double>> bestClusterCoordinates; //Represents best fit of clusters 
	double bestDistance; //Used to calculate if new best set of coordinates
	for (int i=0; i<NUM_ITERATIONS; i++) //Run several random iterations
	{
		//Generate random initial coordinates for cluster
		std::vector<std::vector<double>> clusterCoordinates; 
		for (int j=0; j<numClusters; j++)
		{
			std::vector<double> coords;
			for (int w=0; w<data[0].size()-1; w++) //Generalize to n dimensional data
			{
				double rand_num = (double)rand() / RAND_MAX;
				rand_num = rand_num * (DATA_MAX - DATA_MIN);
				coords.push_back(rand_num);
			}
			clusterCoordinates.push_back(coords);
		}
		int count = 0;
		//Keep reclassifying data and updating clusters until convergence
		do 
		{
			count++;
			classifyData(data, clusterCoordinates);
		} while (!updateClusterLoc(data, clusterCoordinates));
		
		//Check if we have a new best set of clusters
		double averageClusterDistance = computeClusterDistance(data, clusterCoordinates);
		if (averageClusterDistance < bestDistance || i == 0) //If new best or first iteration
		{
			bestClusterCoordinates = clusterCoordinates;
			bestDistance = averageClusterDistance;
		}	
	}

	classifyData(data, bestClusterCoordinates); //Reclassify data to best set of coordinates
	if (ppm) compressImage(data, bestClusterCoordinates, fileName, imageHeader);
	else printResults(data, bestClusterCoordinates);
}

int main(int argc, char *argv[])
{
	srand(time(NULL));
	if (argc != 3)
	{
		std::cout << "Incorrect command line arguments" << std::endl;
		return 0;
	}	
	const int numClusters = std::stoi(argv[1]);
	std::string fileName = argv[2];
	std::ifstream file(fileName);

	std::vector<std::vector<double>> data; //Holds data from file
	std::string line;

	getline(file, line);
	bool ppm = (line == "P3"); //Indicates PPM image

	//Get header info for image file
	std::vector<std::string> imageHeader;
	if (ppm) 
	{
		imageHeader.push_back(line);
		for (int i=0; i<3; i++)
		{
			getline(file, line);
			imageHeader.push_back(line);
		}
	}
	
	//Get input data from file
	while (std::getline(file, line))
	{
		std::vector<double> coords;
		coords.push_back(-1); //First value represents cluster it belongs to -1 means unassigned
		std::stringstream  lineStream(line);
		double coord;
		while (lineStream >> coord)
		{
			coords.push_back(coord);
			if (coords.size() == 4 && ppm)
			{
				data.push_back(coords);
				coords.clear();
				coords.push_back(-1);
			}
		}
		if (!ppm) data.push_back(coords);
	}

	//Run kmeans
	kmeans(data, numClusters, ppm, fileName, imageHeader);
}