#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// structura pentru a retine informatiile unui nod adiacents
typedef struct list {
	int index;
	int dist;
	struct list *next;
} *List, ListNode;

// structura pentru lista de adiacenta
typedef struct node {
	List adj;
	int nr;
	int depth;
} Node;

typedef struct edge {
	int x, y, dist;
} Edge;

int comp_edge(const void *a, const void *b)
{
	int dist_a = ((Edge *)a)->dist;
	int dist_b = ((Edge *)b)->dist;

	return dist_a - dist_b;
}

int comp_int(const void *a, const void *b)
{
	int nra = (*(int *)a);
	int nrb = (*(int *)b);

	return nra - nrb;
}

Node *graph_init(int n)
{
	Node *graph = malloc(n * sizeof(Node));

	int i;
	for (i = 0; i < n; i++) {
		graph[i].adj = NULL;
		graph[i].nr = 0;
	}

	return graph;
}

char **objective_array_init(int n)
{
	char **obj_vect = malloc(n * sizeof(char *));

	int i;
	for (i = 0; i < n; i++) {
		obj_vect[i] = NULL;
	}

	return obj_vect;
}

// cauta in vectorul de obiective cuvantul dat si returneaza pozitia acestuia
// daca il gaseste, altfel returneaza pozitia primei zone libere din vector
// unde obiectivul va fi adaugat
int find_objective_in_array(char **obj_vect, char *obj, int n)
{
	int i = 0;
	while (obj_vect[i]) {
		if (!strcmp(obj_vect[i], obj))
			return i;
		i++;
	}
	return i;
}

// adaug un obiectiv in vector pe o pozitie data
void add_to_obj_array(char **obj_vect, char *obj, int poz)
{
	obj_vect[poz] = malloc((strlen(obj) + 1) * sizeof(char));
	strcpy(obj_vect[poz], obj);
}

// initializeaza o variabila de tip List ce contine datele unui nod adiacent
List adj_node_init(int adj_index, int adj_dist)
{
	List new = malloc(sizeof(ListNode));
	new->index = adj_index;
	new->dist = adj_dist;

	return new;
}

// adaug nod2 in lista de adiacenta a lui nod1
void add_to_adj_list(Node *graph, int nod1, int nod2, int dist)
{
	graph[nod1].nr++;

	List new = adj_node_init(nod2, dist);
	new->next = graph[nod1].adj;
	graph[nod1].adj = new;
}

void free_obj_array(char **obj_array, int n);

// folosind obj_array codific obiectivele date (pozitia din vector este
// codul obiectivului), si folosind aceste codificari creez lista de adiacenta
// a grafului
void create_obj_graph(Node *graph, int n, int m, FILE *in)
{
	char **obj_array = objective_array_init(n);

	int i;
	for (i = 0; i < m; i++) {
		char obj1[50], obj2[50];
		int dist;

		// citesc muchia
		fscanf(in, "%s%s%d", obj1, obj2, &dist);

		// retin pozitia primului obiectiv in vector, aceasta fiind codificarea
		// obiectivului
		int poz1 = find_objective_in_array(obj_array, obj1, n);

		if (obj_array[poz1] == NULL) {
			// nu l-am citit pana acum, deci il adaug obiectivul in vector
			add_to_obj_array(obj_array, obj1, poz1);
		}

		// retin pozitia celui de al doilea obiectiv in vector, aceasta fiind
		// codificarea obiectivului
		int poz2 = find_objective_in_array(obj_array, obj2, n);

		if (obj_array[poz2] == NULL) {
			// nu l-am citit pana acum, deci il adaug obiectivul in vector
			add_to_obj_array(obj_array, obj2, poz2);
		}

		// adaug obj2 in lista de adiacenta a lui obj1
		add_to_adj_list(graph, poz1, poz2, dist);

		// adaug obj1 in lista de adiacenta a lui obj2
		add_to_adj_list(graph, poz2, poz1, dist);
	}

	free_obj_array(obj_array, n);
}

// parcurge o zona conexa si construieste o lista cu muchiile acesteia
void fill_zone(
	Edge **list, int *nr_edges, int *viz, int *t, int poz, Node *graph)
{
	viz[poz] = 1;
	t[poz] = poz;

	List node = graph[poz].adj;
	while (node) {
		// retin intr-o lista toate muchiile din zona pe care o descopar
		// fara sa repet muchiile

		if (node->index > poz) {
			(*nr_edges)++;
			*list = realloc(*list, (*nr_edges) * sizeof(Edge));
			(*list)[*nr_edges - 1].x = poz;
			(*list)[*nr_edges - 1].y = node->index;
			(*list)[*nr_edges - 1].dist = node->dist;
		}
		if (!viz[node->index]) {
			fill_zone(list, nr_edges, viz, t, node->index, graph);
		}
		node = node->next;
	}
}

int kruskal(Edge *list, int n, int m, int *t);

// gaseste fiecare zona conexa din graf si pentru fiecare calculeaza
// costul total minim al drumurilor
void get_zones(int **cost_total, int *zones_nr, Node *graph, int n)
{
	int *viz = calloc(n, sizeof(int));
	int i;
	for (i = 0; i < n; i++) {
		// fiecare nod nevizitat gasit face parte dintr-o zona conexa diferita
		if (!viz[i]) {
			Edge *list = NULL;
			int nr_edges = 0;

			// vectorul de tati initial folosit in alg lui kruskal,
			// initializez fiecare nod dintr-o zona conexa ca fiind
			// propriul sau tata
			int *t = calloc(n, sizeof(int));

			fill_zone(&list, &nr_edges, viz, t, i, graph);

			// sortare lista dupa distanta muchiilor
			qsort(list, nr_edges, sizeof(Edge), comp_edge);

			// cresc numarul de zone conexe si retin pentru aceasta costul
			// total minim pentru renovarea drumurilor
			(*zones_nr)++;
			*cost_total = realloc(*cost_total, (*zones_nr) * sizeof(int));
			(*cost_total)[(*zones_nr) - 1] = kruskal(list, n, nr_edges, t);

			free(t);
			free(list);
		}
	}
	free(viz);
}

int kruskal(Edge *list, int n, int m, int *t)
{
	int i, j;
	int k = 0, s = 0;

	for (i = 0; i < m; i++) {
		// daca nodurile au radacina subarborelui diferita inseamna ca
		// pot sa ii unesc fara sa creez un ciclu
		if (t[list[i].x] != t[list[i].y]) {
			s += list[i].dist;
			k++;

			// actualizez tatal pentru fiecare nod din subarborele celui de
			// al doilea nod din muchie
			int rad_old = t[list[i].y];
			int rad_new = t[list[i].x];
			for (j = 0; j < n; j++) {
				if (t[j] == rad_old) {
					t[j] = rad_new;
				}
			}
		}
	}

	return s;
}

// citeste un nod si returneaza codificarea sa
int get_node_code(FILE *in)
{
	char node[50];
	int poz;

	fscanf(in, "%s", node);

	if (!strcmp(node, "Insula"))
		poz = 0;
	else if (!strcmp(node, "Corabie"))
		poz = 1;
	else {
		strcpy(node, strchr(node, '_') + 1);
		sscanf(node, "%d", &poz);
	}

	return poz;
}

// creeaza lista de adiacenta a grafului din a doua cerinta
void create_island_graph(Node *graph, int n, int m, FILE *in)
{
	int i;
	for (i = 0; i < m; i++) {
		int dist, poz1, poz2;

		poz1 = get_node_code(in);
		poz2 = get_node_code(in);

		fscanf(in, "%d", &dist);

		add_to_adj_list(graph, poz1, poz2, dist);
	}
}

void read_nodes_depth(Node *graph, int n, FILE *in)
{
	char buffer[50];
	int i;
	for (i = 0; i < n; i++)
		fscanf(in, "%s %d", buffer, &graph[i].depth);
}

void DFS(Node *graph, int *viz, int poz)
{
	viz[poz] = 1;

	int i;
	List node = graph[poz].adj;
	while (node) {
		if (!viz[node->index]) {
			DFS(graph, viz, node->index);
		}
		node = node->next;
	}
}

int is_path_to_island(Node *graph, int n, FILE *out)
{
	int *viz = calloc(n, sizeof(int));
	int ok = 1;
	// parcurgem nodurile la care putem ajunge de la corabie
	DFS(graph, viz, 1);
	if (!viz[0]) {
		fprintf(out, "Echipajul nu poate ajunge la insula\n");
		ok = 0;
	}

	free(viz);
	return ok;
}

int is_path_to_boat(Node *graph, int n, FILE *out)
{
	int *viz = calloc(n, sizeof(int));
	int ok = 1;
	// parcurgem nodurile la care putem ajunge de la insula
	DFS(graph, viz, 0);
	if (!viz[1]) {
		fprintf(out,
			"Echipajul nu poate transporta comoara inapoi la "
			"corabie\n");
		ok = 0;
	}

	free(viz);
	return ok;
}

void dijkstra(Node *graph, int n, int *prev)
{
	int i;
	// vector care retine scorul minim posibil al drumului de la insula
	// la fiecare nod
	float *scores = malloc(n * sizeof(float));
	int *viz = calloc(n, sizeof(int));

	// marchez scorul minim total pana la fiecare nod in afara de insula cu
	// infinit
	for (i = 1; i < n; i++)
		scores[i] = __FLT_MAX__;
	scores[0] = 0;

	int min_node;
	while (1) {
		float min_score = __FLT_MAX__;

		// iau nodul nevizitat cu scorul total al drumului pana la el minim
		for (i = 0; i < n; i++)
			if (!viz[i] && scores[i] < min_score) {
				min_score = scores[i];
				min_node = i;
			}

		// daca a fost extras nodul 1 (Corabia) inseamna ca drumul cel mai
		// eficient a fost gasit
		if (min_node == 1)
			break;

		viz[min_node] = 1;
		List node = graph[min_node].adj;
		// parcurg nodurile adiacente nodului extras
		while (node) {
			int adj_node = node->index;

			// actualizez scorul drumului minim pentru fiecare nod adiacent
			// care nu a fost inca vizitat
			if (!viz[adj_node]) {
				int adj_node_dist = node->dist;
				int adj_node_depth = graph[adj_node].depth;
				float temp_score
					= scores[min_node] + adj_node_dist * 1.0 / adj_node_depth;

				if (temp_score < scores[adj_node]) {
					scores[adj_node] = temp_score;
					prev[adj_node] = min_node;
				}
			}

			node = node->next;
		}
	}

	free(scores);
	free(viz);
}

// functie ce parcurge recursiv vectorul prev pana ajunge la insula, apoi
// retine drumul in path
void get_path(int node, int *prev, int **path, int *path_len)
{
	if (node != 0)
		get_path(prev[node], prev, path, path_len);

	(*path_len)++;
	*path = realloc(*path, (*path_len) * sizeof(int));
	(*path)[(*path_len) - 1] = node;
}

int get_path_cost(Node *graph, int *path, int path_len)
{
	int i, cost_t = 0;
	for (i = 0; i < path_len - 1; i++) {
		int node1 = path[i], node2 = path[i + 1];

		List node = graph[node1].adj;
		// caut costul arcului de la nodul curent(nod1) la urmatorul din
		// drum(nod2) si il adaug in costul total
		while (node->index != node2)
			node = node->next;

		cost_t += node->dist;
	}

	return cost_t;
}

int get_max_weight(Node *graph, int *path, int path_len)
{
	int i, max_weight = __INT_MAX__;
	for (i = 1; i < path_len - 1; i++) {
		int node = path[i];
		if (graph[node].depth < max_weight)
			max_weight = graph[node].depth;
	}

	return max_weight;
}

int get_trip_number(int treasure_weight, int max_weight)
{
	int times = treasure_weight / max_weight;
	// daca produsul este mai mic, inseamna ca dupa times drumuri a mai
	// ramas din comoara pe insula, fiind necesar inca un drum
	if (times * max_weight < treasure_weight)
		times++;

	return times;
}

void print_zones_cost(int *dist_min, int zones_nr, FILE *out)
{
	fprintf(out, "%d\n", zones_nr);
	int i;
	for (i = 0; i < zones_nr; i++)
		fprintf(out, "%d\n", dist_min[i]);
}

void print_path(int *path, int path_len, FILE *out)
{
	int i;
	fprintf(out, "Insula ");
	for (i = 1; i < path_len - 1; i++)
		fprintf(out, "Nod_%d ", path[i]);
	fprintf(out, "Corabie\n");
}

void free_graph(Node *graph, int n)
{
	int i;
	for (i = 0; i < n; i++) {
		List node = graph[i].adj;
		while (node) {
			List aux = node;
			node = node->next;
			free(aux);
		}
	}
	free(graph);
}

void free_obj_array(char **obj_array, int n)
{
	int i;
	for (i = 0; i < n; i++) {
		free(obj_array[i]);
	}
	free(obj_array);
}

int main(int argc, char *argv[])
{
	FILE *in = fopen("tema3.in", "r");
	FILE *out = fopen("tema3.out", "w");

	int n, m;
	fscanf(in, "%d%d", &n, &m);
	Node *graph = graph_init(n);

	if (argc == 2) {
		if (!strcmp(argv[1], "1")) {
			// trebuie rezolvata prima cerinta

			create_obj_graph(graph, n, m, in);

			// vector in care retin costul total minim al drumurilor ce trebuie
			// renovate pentru fiecare zona conexa
			int *cost_total = NULL;
			int zones_nr = 0;
			get_zones(&cost_total, &zones_nr, graph, n);

			// sortez distantele obtinute
			qsort(cost_total, zones_nr, sizeof(int), comp_int);

			print_zones_cost(cost_total, zones_nr, out);

			free(cost_total);
		} else if (!strcmp(argv[1], "2")) {
			// trebuie rezolvata a doua cerinta
			int treasure_weight;

			create_island_graph(graph, n, m, in);

			read_nodes_depth(graph, n, in);

			fscanf(in, "%d", &treasure_weight);

			if (is_path_to_island(graph, n, out)) {
				if (is_path_to_boat(graph, n, out)) {

					int *prev = calloc(n, sizeof(int));
					dijkstra(graph, n, prev);

					int *path = NULL;
					int path_len = 0;
					get_path(1, prev, &path, &path_len);

					print_path(path, path_len, out);

					int cost_t = get_path_cost(graph, path, path_len);
					fprintf(out, "%d\n", cost_t);

					int max_weight = get_max_weight(graph, path, path_len);
					fprintf(out, "%d\n", max_weight);

					int times = get_trip_number(treasure_weight, max_weight);
					fprintf(out, "%d\n", times);

					free(prev);
					free(path);
				}
			}
		}
	}

	free_graph(graph, n);
	fclose(in);
	fclose(out);
	return 0;
}