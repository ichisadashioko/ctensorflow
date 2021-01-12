#define _CRT_SECURE_NO_DEPRECATE
#include <stdio.h>
#include <stdlib.h>
#include <direct.h>

#include <tensorflow/c/c_api.h>
#pragma comment(lib, "tensorflow.lib")

void free_buffer(void* data, size_t length) {
	free(data);
}

TF_Buffer* read_file(const char* fpath) {
	FILE* fp = fopen(fpath, "rb");

	if (fp == NULL) {
		printf("Failed to open file %s\n", fpath);
		printf("%s:%d\n", __FILE__, __LINE__);
		exit(1);
	}

	fseek(fp, 0, SEEK_END);
	long fsize = ftell(fp);
	fseek(fp, 0, SEEK_SET); // same as rewind(f)

	void* data = malloc(fsize);

	if (data == NULL) {
		printf("Failed to call malloc!\n");
		printf("%s:%d\n", __FILE__, __LINE__);
		exit(1);
	}

	fread(data, fsize, 1, fp);
	fclose(fp);

	TF_Buffer* buf = TF_NewBuffer();
	buf->data = data;
	buf->length = fsize;
	buf->data_deallocator = free_buffer;
	return buf;
}

int main()
{
	char* buffer;

	// Get the current working directory
	if ((buffer = _getcwd(NULL, 0)) == NULL) {
		perror("_getcwd error");
	}
	else {
		printf("cwd: %s\n", buffer);
		free(buffer);
	}


	printf("Hello from TensorFlow C library version %s\n", TF_Version());

	TF_Buffer* graph_def = read_file("tensorflow_inception_graph.pb");
	TF_Graph* graph = TF_NewGraph();

	// Import graph_def into graph
	TF_Status* status = TF_NewStatus();
	TF_ImportGraphDefOptions* opts = TF_NewImportGraphDefOptions();
	TF_GraphImportGraphDef(graph, graph_def, opts, status);
	TF_DeleteImportGraphDefOptions(opts);

	if (TF_GetCode(status) != TF_OK) {
		//fprintf(stderr, "ERROR: Unable to import graph %s\n", TF_Message(status));
		printf("ERROR: Unable to import graph %s\n", TF_Message(status));
		return 1;
	}

	//fprintf(stdout, "Successfully imported graph\n");
	printf("Successfully imported graph\n");
	TF_DeleteStatus(status);
	TF_DeleteBuffer(graph_def);

	// Use the graph

	TF_DeleteGraph(graph);
	return 0;
	return 0;
}
