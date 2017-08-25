#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <sstream> //Handling input
#include <stdlib.h> //Random numbers
#include <math.h> //Power function
#include <iomanip> //Setprecision for printing results
#include <climits>

using namespace std;

/********************* NOTES *************************

- If 2 coords are same distance from cluster it will 
  be classified to the lower valued cluster.

- Cluster initialization is random within defined
  data range

*****************************************************/


//Classify's data to it's nearest cluster, the cluster it belongs to is stored in first element of coordinate vector
void classifyData(vector<vector<double>> &data, vector<vector<double>> &clusterCoordinates)
{
	for (int i=0; i<data.size(); i++)
	{
		double minDistance = INT_MAX;
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
bool updateClusterLoc(vector<vector<double>> &data, vector<vector<double>> &clusterCoordinates)
{
	vector<vector<double>> prevClusterCoordinates = clusterCoordinates;
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
double computeClusterDistance(vector<vector<double>> &data, vector<vector<double>> &clusterCoordinates)
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
void printResults(vector<vector<double>> &data, vector<vector<double>> &clusterCoordinates)
{
	for (int i=0; i<clusterCoordinates.size(); i++)
	{
		cout << "Cluster coordinate " << i << ": ";
		for (int j=0; j<clusterCoordinates[i].size(); j++)
		{
			cout << clusterCoordinates[i][j] << " ";
		}
		cout << endl;
	}
	cout << fixed << setprecision(2);
	for (int i=0; i<data.size(); i++)
	{
		cout << "Data " << i << " in cluster " << data[i][0] << ", coordinates: ";
		for (int j=1; j<data[i].size(); j++)
		{
			cout << data[i][j] << " ";
		}
		cout << endl;
	}
}

//Outputs data in correct format to image file
void compressImage	(vector<vector<double>> &data, 
					vector<vector<double>> &clusterCoordinates, 
					string fileName,
					vector<string> &imageHeader)
{
	string outputFile = fileName + "-compressed.ppm";
	ofstream output(outputFile);
	for (int i=0; i<imageHeader.size(); i++)
	{
		output << imageHeader[i] << endl;
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
		output << endl;
	}
}

//Runs kmeans clustering on data vector, if ppm outputs compressed file
void kmeans	(vector<vector<double>> &data, 
			int numClusters, 
			bool ppm, 
			string fileName,
			vector<string> &imageHeader,
			int num_iterations)
{
	//Get the max and min values of dataset
	int data_max = INT_MIN;
	int data_min = INT_MAX;
	for (int i=0; i<data.size(); i++)
	{
		for (int j=0; j<data[i].size(); j++)
		{
			if (data[i][j] > data_max) data_max = data[i][j];
			if (data[i][j] < data_min) data_min = data[i][j];
		}
	}
		std::cout << data_max << std::endl;

	vector<vector<double>> bestClusterCoordinates; //Holds best fit of clusters 
	double bestDistance; //Used to calculate if new best set of coordinates
	for (int i=0; i<num_iterations; i++) //Run several random iterations
	{
		//Generate random initial coordinates for cluster
		vector<vector<double>> clusterCoordinates; 
		for (int j=0; j<numClusters; j++)
		{
			vector<double> coords;
			for (int w=0; w<data[0].size()-1; w++) //Generalize to n dimensional data
			{
				double rand_num = (double)rand() / RAND_MAX;
				rand_num = rand_num * (data_max - data_max);
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
	if (argc != 4)
	{
		cout << "Incorrect command line arguments" << endl;
		return 0;
	}	
	const int numClusters = stoi(argv[1]);
	const int num_iterations = stoi(argv[3]);
	string fileName = argv[2];
	ifstream file(fileName);

	vector<vector<double>> data; //Holds data from file
	string line;

	getline(file, line);
	bool ppm = (line == "P3"); //Indicates PPM image

	//Get header info for image file
	vector<string> imageHeader;
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
	while (getline(file, line))
	{
		vector<double> coords;
		coords.push_back(-1); //First value represents cluster it belongs to -1 means unassigned
		stringstream  lineStream(line);
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
	kmeans(data, numClusters, ppm, fileName, imageHeader, num_iterations);
}