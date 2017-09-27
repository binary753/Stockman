#ifndef ASTAR_H
#define ASTAR_H

#include <string>
#include <vector>

#include "NavigationMesh.h"
#include "PASVF.h"

#include <Entity\Entity.h>

namespace Logic
{
	class AStar
	{
		public:
			struct Node {
				int triangleID;
				btVector3 position;
			};

			// to calc path testing rn
			struct NavNode
			{
				bool open; // for testing
				int nodeIndex; // index in nav mesh
				float f, g; // cost to node

				bool Compare(NavNode const &n, NavNode const &n2) {
					return n.f > n2.f;
				}
			};

			// singleton for the moment
			static AStar& singleton()
			{
				static AStar aStar("");
				return aStar;
			}
		private:
			std::string file;
			std::vector<Node> nodes; //testing
			NavigationMesh navigationMesh;
		
			float heuristic(DirectX::SimpleMath::Vector3 &from,
				DirectX::SimpleMath::Vector3 &to) const;
			void generateNodesFromFile();
		public:
			// string for the offline loaded nav mesh
			AStar(std::string file);
			~AStar();

			Node getNextNode(Entity const &enemy, Entity const &target);
			void reconstructPath(std::vector<NavNode> &navNodes,
				std::vector<DirectX::SimpleMath::Vector3> &nodes);

			// iniate the nodes
			void generateNavigationMesh();
	};
}
#endif