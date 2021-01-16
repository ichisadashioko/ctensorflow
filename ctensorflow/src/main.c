#define _CRT_SECURE_NO_DEPRECATE
#include <direct.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include <tensorflow/c/c_api.h>
#pragma comment(lib, "tensorflow.lib")

#define SAVED_MODEL_NUM_TAGS 1

void doNothingDeallocator(void* data, size_t len, void* arg) {}

int main(int argc, char* argv[]) {
	for (int i = 0; i < argc; i++) {
		printf("%d - %s\n", i, argv[i]);
	}

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

	// look at the output of cwd above and move the SavedModel directory to where is appropriate while debuging with Visual Studio / your IDE
	// TODO: use argv[0] to resolve the SavedModel path
	const char* SAVED_MODEL_DIR = "etl9b.model.tensorflow.savedmodel/";
	const char* SAVED_MODEL_TAG = "serve";

	TF_Graph* graph = TF_NewGraph();
	TF_Status* tfStatus = TF_NewStatus();
	TF_SessionOptions* tfSessOptions = TF_NewSessionOptions();
	TF_Buffer* tfRunOptions = NULL;

	TF_Session* tfSession = TF_LoadSessionFromSavedModel(
		tfSessOptions, tfRunOptions, SAVED_MODEL_DIR, &SAVED_MODEL_TAG,
		SAVED_MODEL_NUM_TAGS, graph, NULL, tfStatus);

	if (TF_GetCode(tfStatus) == TF_OK) {
		printf("TF_LoadSessionFromSavedModel OK\n");

		// get input tensor
		int numInputs = 1;
		TF_Output* inputTensor = malloc(sizeof(TF_Output) * numInputs);
		if (inputTensor == NULL) {
			printf("ERROR: %s:%d\n", __FILE__, __LINE__);
			printf("malloc failed!\n");
		}
		else {
			TF_Output t0 = { .oper = TF_GraphOperationByName(
								graph, "serving_default_conv2d_00_input"),
							.index = 0 };

			if (t0.oper == NULL) {
				printf("ERROR: %s:%d\n", __FILE__, __LINE__);
				printf("TF_GraphOperationByName failed!\n");
			}
			else {
				printf("TF_GraphOperationByName OK\n");

				inputTensor[0] = t0;

				// get output tensor
				int numOutputs = 1;
				TF_Output* outputTensor = malloc(sizeof(TF_Output) * numOutputs);

				if (outputTensor == NULL) {
					printf("ERROR: %s:%d\n", __FILE__, __LINE__);
					printf("malloc failed!\n");
				}
				else {
					TF_Output t2 = {
						.oper = TF_GraphOperationByName(graph, "StatefulPartitionedCall"),
						.index = 0 };

					if (t2.oper == NULL) {
						printf("ERROR: %s:%d\n", __FILE__, __LINE__);
						printf("TF_GraphOperationByName failed!\n");
					}
					else {
						printf("TF_GraphOperationByName OK\n");

						outputTensor[0] = t2;

						// allocate data for inputs and outputs
						TF_Tensor** inputValues =
							(TF_Tensor**)malloc(sizeof(TF_Tensor*) * numInputs);
						if (inputValues == NULL) {
							printf("ERROR: %s:%d\n", __FILE__, __LINE__);
							printf("malloc failed!\n");
						}
						else {
							TF_Tensor** outputValues =
								(TF_Tensor**)malloc(sizeof(TF_Tensor*) * numOutputs);
							if (outputValues == NULL) {
								printf("ERROR: %s:%d\n", __FILE__, __LINE__);
								printf("malloc failed!\n");
							}
							else {
								int ndimsOfInputs = 4; // (-1, 64, 64, 1)
								int64_t inputDims[] = { 1, 64, 64, 1 };

								// sizeof("pixel_data_type") * num_input_images * height * width * num_channels
								size_t inputDataByteCount = sizeof(float) * 1 * 64 * 64 * 1;
								float* sampleInputData = malloc(inputDataByteCount);

								if (sampleInputData == NULL) {
									printf("ERROR: %s:%d\n", __FILE__, __LINE__);
									printf("malloc failed!\n");
								}
								else {
									// zeroing out memory
									memset(sampleInputData, 0, inputDataByteCount);

									/*
									* TF_NewTensor function signature
									*
									* TF_CAPI_EXPORT extern TF_Tensor* TF_NewTensor(
									*	TF_DataType, const int64_t* dims, int num_dims, void* data, size_t len,
									*	void (*deallocator)(void* data, size_t len, void* arg),
									*	void* deallocator_arg);
									*/
									TF_Tensor* floatInputTensor = TF_NewTensor(
										TF_FLOAT,              // TF_DataType
										inputDims,             // const int64_t* dims
										ndimsOfInputs,         // int num_dims
										sampleInputData,       // void* data
										inputDataByteCount,    // size_t len
										&doNothingDeallocator, // void* arg
										0                      // void* deallocator_arg
									);

									if (floatInputTensor == NULL) {
										printf("ERROR: %s:%d\n", __FILE__, __LINE__);
										printf("TF_NewTensor failed!\n");
									}
									else {
										printf("TF_NewTensor OK\n");

										inputValues[0] = floatInputTensor;

										// run the session
										TF_SessionRun(tfSession, NULL, inputTensor, inputValues,
											numInputs, outputTensor, outputValues,
											numOutputs, NULL, 0, NULL, tfStatus);

										if (TF_GetCode(tfStatus) == TF_OK) {
											printf("TF_SessionRun OK\n");

											void* outputDataVoidPtr = TF_TensorData(outputValues[0]);

											float* outputData = (float*)outputDataVoidPtr;
											float maxOutputValue = outputData[0];
											int maxOutputIdx = 0;

											// my model's output will have shape of (1, 3001)
											for (int i = 1; i < 3001; i++) {
												if (outputData[i] > maxOutputValue) {
													maxOutputValue = outputData[i];
													maxOutputIdx = i;
												}
											}

											printf("maxOutputIdx: %d\n", maxOutputIdx);
										}
										else {
											printf("ERROR: %s:%d\n", __FILE__, __LINE__);
											printf("TF_SessionRun failed\n");
											printf("%s\n", TF_Message(tfStatus));
										}
									}

									free(sampleInputData);
								}

								free(outputValues);
							}

							free(inputValues);
						}
					}

					free(outputTensor);
				}
			}

			free(inputTensor);
		}
	}
	else {
		printf("ERROR: %s:%d\n", __FILE__, __LINE__);
		printf("%s\n", TF_Message(tfStatus));
	}

	TF_DeleteGraph(graph);
	TF_DeleteSession(tfSession, tfStatus);
	TF_DeleteSessionOptions(tfSessOptions);
	TF_DeleteStatus(tfStatus);

	return 0;
}
