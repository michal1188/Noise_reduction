
#include "PNG.h"
#include <CL/cl.hpp>


#define MAX_SOURCE_SIZE (0x100000)

using namespace std;
using namespace cl;

char* value;
std::size_t valueSize;
cl_uint maxComputeUnits;
int DEVICE_TYPE;
cl_int cli_err_num;
cl_uint clui_num_platforms = 0;
cl_uint clui_num_devices = 0;
cl_platform_id* opencl_platforms = NULL;
cl_device_id* opencl_devices = NULL;
cl_context cl_compute_context = NULL;
cl_command_queue cl_compute_command_queue = NULL;
cl_program Cl_kernel_program;

const char* attributeNames[5] = { "Name", "Vendor",
	"Version", "Profile", "Extensions" };
const cl_platform_info attributeTypes[5] = { CL_PLATFORM_NAME, CL_PLATFORM_VENDOR,
	CL_PLATFORM_VERSION, CL_PLATFORM_PROFILE, CL_PLATFORM_EXTENSIONS };
const int attributeCount = sizeof(attributeNames) / sizeof(char*);
char* info;
std::size_t infoSize;
cl_device_type dt;

float* makeMatrix(int base_matrix_size, float sigma, float* filter_values) {
	int x, y;
	float center = base_matrix_size / 2.0f;
	float total_sum = 0.0f;

	for (x = 0; x < base_matrix_size; x++) {
		for (y = 0; y < base_matrix_size; y++)
		{
			filter_values[y*base_matrix_size + x] = exp((((x - center) * (x - center) + (y - center) * (y - center)) / (2.0f * sigma * sigma)) * -1.0f) / (2.0f * CL_M_PI * sigma * sigma);

			total_sum += filter_values[y * base_matrix_size + x];
		}
	}
	for (x = 0; x < base_matrix_size * base_matrix_size; x++)
	{
		filter_values[x] = filter_values[x] / total_sum;

	}
	//FILE* f2;
	//f2 = fopen("zapisywanie.txt", "w");
	/*
	printf("Macierz filtru Gaussowskiego: \n");
	for (x = 0; x < base_matrix_size; x) {
		for (y = 0; y < base_matrix_size; y++) {
			//fprintf(f2, " % dn % d", filter_values);
			printf("%f", filter_values[y * base_matrix_size + x]);
		}
			printf("\n");
	}
	//fclose(f2);
	*/
	return filter_values;

}




void addLines(const std::string sciezka, const std::string sciezka2)
{
	std::vector < std::string > vec;
	ifstream in(sciezka.c_str());
	ifstream in2(sciezka2.c_str());
	string tmp;

	while (getline(in2, tmp)) vec.push_back(tmp);

	in2.close();
	while (getline(in, tmp)) vec.push_back(tmp);

	in.close();

	ofstream out(sciezka2.c_str());

	for (std::size_t i = 0; i < vec.size(); ++i) {


		out << vec[i] << endl;
	}
	out.close();
}

int main(int arg, char* args[]) {
	cli_err_num = CL_SUCCESS;
	cli_err_num = clGetPlatformIDs(0, NULL, &clui_num_platforms);

	if (cli_err_num != CL_SUCCESS) {

		printf("Brak dostepu OPENCL!!!\n");
		return 0;
	}
	printf("Dostepna liczba platform OPENCL: (%d).\n", clui_num_platforms);

	opencl_platforms = (cl_platform_id*)malloc(clui_num_platforms * sizeof(cl_platform_id));
	if (opencl_platforms == NULL) {
		printf("Brak alokacji pamieci dla platform OpenCL!!\n");
		return 0;
	}

	cli_err_num = clGetPlatformIDs(clui_num_platforms, opencl_platforms, NULL);
	if (cli_err_num != CL_SUCCESS) {

		free((void*)opencl_platforms);
		opencl_platforms = NULL;
		return 0;
	}


	// for each platform print all attributes
	for (int i = 0; i < clui_num_platforms; i++) {

		printf("\n %d. Platform \n", i);

		for (int j = 0; j < attributeCount; j++) {

			// get platform attribute value size
			clGetPlatformInfo(opencl_platforms[i], attributeTypes[j], 0, NULL, &infoSize);
			info = (char*)malloc(infoSize);

			// get platform attribute value
			clGetPlatformInfo(opencl_platforms[i], attributeTypes[j], infoSize, info, NULL);

			printf("  %d.%d %-11s: %s\n", i, j + 1, attributeNames[j], info);
			free(info);

		}

		printf("\n");

	}

	int x;
	printf("Wybierz platforme: \n");
	scanf("%d", &x);
	fflush(stdin);

	if (x > clui_num_platforms - 1) {
		printf("Error:Nie ma platformy o takim numerze porzadkowym! \n ");
		return EXIT_FAILURE;
	}

	clGetDeviceIDs(opencl_platforms[x], CL_DEVICE_TYPE_ALL, 0, NULL, &clui_num_devices);
	opencl_devices = (cl_device_id*)malloc(sizeof(cl_device_id) * clui_num_devices);
	cli_err_num = clGetDeviceIDs(opencl_platforms[x], CL_DEVICE_TYPE_ALL, clui_num_devices, opencl_devices, NULL);

	if (cli_err_num != CL_SUCCESS) {
		printf("Error: Nie udalo się odczytac infrmacji o urzadzeniach obliczeniowych! \n");
		return EXIT_FAILURE;
	}

	for (int j = 0; j < clui_num_devices; j++) {

		// print device name
		clGetDeviceInfo(opencl_devices[j], CL_DEVICE_NAME, 0, NULL, &valueSize);
		value = (char*)malloc(valueSize);
		clGetDeviceInfo(opencl_devices[j], CL_DEVICE_NAME, valueSize, value, NULL);
		printf("%d. Device: %s\n", j, value);
		free(value);

		// print hardware device version
		clGetDeviceInfo(opencl_devices[j], CL_DEVICE_VERSION, 0, NULL, &valueSize);
		value = (char*)malloc(valueSize);
		clGetDeviceInfo(opencl_devices[j], CL_DEVICE_VERSION, valueSize, value, NULL);
		printf(" %d.%d Hardware version: %s\n", j, 1, value);
		free(value);

		// print software driver version
		clGetDeviceInfo(opencl_devices[j], CL_DRIVER_VERSION, 0, NULL, &valueSize);
		value = (char*)malloc(valueSize);
		clGetDeviceInfo(opencl_devices[j], CL_DRIVER_VERSION, valueSize, value, NULL);
		printf(" %d.%d Software version: %s\n", j, 2, value);
		free(value);

		// print c version supported by compiler for device
		clGetDeviceInfo(opencl_devices[j], CL_DEVICE_OPENCL_C_VERSION, 0, NULL, &valueSize);
		value = (char*)malloc(valueSize);
		clGetDeviceInfo(opencl_devices[j], CL_DEVICE_OPENCL_C_VERSION, valueSize, value, NULL);
		printf(" %d.%d OpenCL C version: %s\n", j, 3, value);
		free(value);


		// print parallel compute units
		clGetDeviceInfo(opencl_devices[j], CL_DEVICE_MAX_COMPUTE_UNITS,
			sizeof(maxComputeUnits), &maxComputeUnits, NULL);
		printf(" %d.%d Parallel compute units: %d\n", j, 4, maxComputeUnits);

		// print type device
		clGetDeviceInfo(opencl_devices[j], CL_DEVICE_TYPE,
			sizeof(dt), &dt, NULL);
		if (dt == 2) {

			printf(" %d.%d CL_DEVICE_TYPE: %s\n", j, 5, "CPU");
			DEVICE_TYPE = 1;
		}
		else if (dt == 4) {
			printf(" %d.%d CL_DEVICE_TYPE: %s\n", j, 5, "GPU");
			DEVICE_TYPE = 2;
		}
		else {
			printf(" %d.%d CL_DEVICE_TYPE: %s\n", j, 5, "ACCELERATOR OR  DEFAULT");
			DEVICE_TYPE = 3;
		}
		/*
		clGetDeviceInfo(opencl_devices[j], CL_DEVICE_TYPE, sizeof(cl_device_type), &dt, NULL);
		printf(" CL_DEVICE_TYPE: %s\n",
			dt& CL_DEVICE_TYPE_GPU ? " GPU" : "",
			dt & CL_DEVICE_TYPE_CPU ? " CPU" : "",

			dt & CL_DEVICE_TYPE_ACCELERATOR ? " ACCELERATOR" : "",
			dt & CL_DEVICE_TYPE_DEFAULT ? " DEFAULT" : "");
			*/
	}




	int y;
	printf("Wybierz urzadzenie: \n");
	scanf("%d", &y);
	fflush(stdin);

	if (y > clui_num_devices - 1) {
		printf("Error:Nie ma urzadzenia o takim numerze porzadkowym! \n ");
		return EXIT_FAILURE;
	}

	cl_compute_context = clCreateContext(NULL, clui_num_devices, &opencl_devices[y], NULL, NULL, &cli_err_num);
	if (!cl_compute_context) {
		printf("Error: Nie udalo sie utworzyc kontekstu obliczeniowgo! \n ");
		return EXIT_FAILURE;
	}

	cl_compute_command_queue = clCreateCommandQueue(cl_compute_context, opencl_devices[y], CL_QUEUE_PROFILING_ENABLE, &cli_err_num);
	if (!cl_compute_command_queue) {
		printf("Error: Nie udalo sie utworzyc kolejki polecen! \n ");
		return EXIT_FAILURE;
	}

	char  photo[100];

	printf("Wpisz nazwe  fotografii  w formacie .png do przetworzenia:   \n");
	scanf("%100s", &photo);
	fflush(stdin);
	string p = photo;
	//load our image
	PNG inPng(photo);

	//store width and height so we can use them for our output image later
	const unsigned int w = inPng.w;
	const unsigned int h = inPng.h;


	cl_image_format image_format;
	image_format.image_channel_data_type = CL_UNORM_INT8;
	image_format.image_channel_order = CL_RGBA;

	cl_mem  input_image;
	cl_mem  output_image;

	//input image

	float Tab_scale_factor[7] = { 0.2, 0.5,0.7,1,2,5,10};
	float scale_factor;
	float fwn;
	float fhn;
	unsigned int  scale_w;
	unsigned int  scale_h;
	std::size_t globalWorkSize[2];

	float maska;
	printf("Wpisz rozmiar maski dla rozmycia(np: 7 -> 7x7): \n");
	scanf("%f", &maska);
	
	printf("\n MASKA - %f\n", maska);
	const int FILTER_WINDOW_SIZE = ((int)maska-1)/2;
	printf("\n MASKA - %d\n", FILTER_WINDOW_SIZE);
	float* filterValues = new float[((FILTER_WINDOW_SIZE * 2) + 1) * ((FILTER_WINDOW_SIZE * 2) + 1)];
	float* mask = makeMatrix((2 * FILTER_WINDOW_SIZE + 1), 1000.0f, filterValues);
	char preprocesor_options[512];
	sprintf(preprocesor_options, "-DFILTER_WINDOW=%d", FILTER_WINDOW_SIZE, "\n");

	char* buf = (char*)malloc(MAX_SOURCE_SIZE);
	int count;
	FILE* fp;
	FILE* wyniki;

	wyniki = fopen("950.txt", "w");
	if (wyniki == NULL) {
		perror("fopen failed");
		exit(1);
	}
	fprintf(wyniki, "Maska rozmycia 51x51\n" );

	char* source_str;
	std::size_t source_size;

	cl_event timing_event;
	cl_ulong time_start = (cl_ulong)0;
	cl_ulong  time_end = (cl_ulong)0;
	float read_time;
	cl_build_status build_status;
	cl_kernel kernel;

	cl::size_t<3> origin;
	cl::size_t<3> size;
	PNG outPng;

	cl_event timing_event2;
	cl_ulong time_start2 = (cl_ulong)0;
	cl_ulong  time_end2 = (cl_ulong)0;
	float read_time2;


	cl_mem output_image2;
	cl_mem input_image2;

	cl_kernel kernel2;

	std::size_t globalWorkSize2[2];
	PNG outPng2;
	unsigned char* tmp2;
	unsigned char* tmp;
	string resize;
	//skalowanie
	/*float scale;
	printf("Wpisz wspolczynnik skali:\n");
	printf("- 1 obraz w rozmiarze orginalu \n");
	printf("- >1  obraz wiekszy od orginalu \n");
	printf("- <1  obraz mniejszy od orginalu \n");
	scanf("%f", &scale);
		fflush(stdin);
	*/
	for (int i = 0; i < sizeof(Tab_scale_factor) / sizeof(float);i++) {
	scale_factor = Tab_scale_factor[i];
	resize = to_string(Tab_scale_factor[i]);
	fwn = 1.0f / (w * scale_factor);
	fhn = 1.0f / (h * scale_factor);

	scale_w = w * scale_factor;
	scale_h = h * scale_factor;

	 input_image =  clCreateImage2D(cl_compute_context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, &image_format, w, h, 0, &inPng.data[0], &cli_err_num);
	 output_image = clCreateImage2D(cl_compute_context, CL_MEM_WRITE_ONLY, &image_format, scale_w, scale_h, 0, NULL, &cli_err_num);

	


	globalWorkSize[0] = scale_w;
	globalWorkSize[1] = scale_h;



	printf("\n etap1 ");
	count = sprintf(buf, "#define WNF ");
	count += sprintf(buf + count, "%g ", fwn);
	count += sprintf(buf + count, "\n#define HNF ", fhn);
	count += sprintf(buf + count, "%g ", fhn);
	count += sprintf(buf + count, "\n#define DEVICE_TYPE ", DEVICE_TYPE);
	count += sprintf(buf + count, " %d ", DEVICE_TYPE);
	count += sprintf(buf + count, "\n#define MASK ");
	for (int i = 0; i < ((2 * FILTER_WINDOW_SIZE + 1)* (2 * FILTER_WINDOW_SIZE + 1)); i++) {
		count += sprintf(buf + count, "%g, ", mask[i]);

	}
	count += sprintf(buf + count, "\n");




	//odczytanie pliku
	fp = fopen("Kernel.txt", "r+");
	if (!fp) {
		fprintf(stderr, "Nie mozna zaladowac kerneli.\n");
		exit(1);
	}
	source_str = (char*)malloc(MAX_SOURCE_SIZE);
	source_size = fread(source_str, 1, MAX_SOURCE_SIZE, fp);
	fclose(fp);

	//dopisanie do pliku
	fp = fopen("cl_kernel.cl", "w+");
	fwrite(buf, sizeof(char), strlen(buf), fp);
	fclose(fp);

	addLines("Kernel.txt", "cl_kernel.cl");

	//ponowne odczytanie pliku i wyznaczenie rozmiaru dla pamięci 
	fp = fopen("cl_kernel.cl", "r+");
	if (!fp) {
		fprintf(stderr, "Nie mozna zaladowac kerneli.\n");
		exit(1);
	}
	source_str = (char*)malloc(MAX_SOURCE_SIZE);
	source_size = fread(source_str, 1, MAX_SOURCE_SIZE, fp);
	fclose(fp);

	printf("\n etap2 ");

	Cl_kernel_program = clCreateProgramWithSource(cl_compute_context, 1, (const char**)&source_str, (const std::size_t*) & source_size, &cli_err_num);
	if (!Cl_kernel_program) {
		printf("Error: Nie udalo się utworzyc programu obliczeniowego! Kod =%d\n", cli_err_num);
		return EXIT_FAILURE;
	}
	cli_err_num = clBuildProgram(Cl_kernel_program, clui_num_devices, opencl_devices, preprocesor_options, NULL, NULL);


	cli_err_num = clGetProgramBuildInfo(Cl_kernel_program, opencl_devices[y], CL_PROGRAM_BUILD_STATUS, sizeof(cl_build_status), &build_status, NULL);

	if (build_status != CL_SUCCESS) {
		printf("\n BUILD ERROR \n", build_status);
		char* build_log;
		std::size_t ret_val_size;
		cli_err_num = clGetProgramBuildInfo(Cl_kernel_program, opencl_devices[y], CL_PROGRAM_BUILD_LOG, 0, NULL, &ret_val_size);
		build_log = (char*)malloc(ret_val_size + 1);
		cli_err_num = clGetProgramBuildInfo(Cl_kernel_program, opencl_devices[y], CL_PROGRAM_BUILD_LOG, ret_val_size, build_log, NULL);

		build_log[ret_val_size] = '\0';
		printf("\n %s \n", build_log);
		return 1;
	}
	else {
		printf("\n Gotowe - utworzono obiekt programu dla kontekstu obliczeniowego resize - %f\n", scale_factor );
	}
	//set the kernel arguments

	kernel = clCreateKernel(Cl_kernel_program, "scaleKernel_f", &cli_err_num);
	if (cli_err_num != CL_SUCCESS) {
		printf("brak kodu kernela \n");
		return 1;
	}

	cli_err_num = clSetKernelArg(kernel, 0, sizeof(cl_mem), &input_image);
	cli_err_num = clSetKernelArg(kernel, 1, sizeof(cl_mem), &output_image);

	if (cli_err_num != CL_SUCCESS) {
		printf("Nie udało się ustalić parametrów kernela! \n");
		return EXIT_FAILURE;

	}
	else {

		cli_err_num = clSetKernelArg(kernel, 0, sizeof(cl_mem), &input_image);
		cli_err_num = clSetKernelArg(kernel, 1, sizeof(cl_mem), &output_image);
		if (cli_err_num != CL_SUCCESS) {
			printf("Nie udało się ustalić parametrów kernela! \n");
			return EXIT_FAILURE;
		}
	}

	cli_err_num = clEnqueueNDRangeKernel(cl_compute_command_queue, kernel, 2, NULL, globalWorkSize, NULL, 0, NULL, &timing_event);
	//wait for kernel to finish
	clFinish(cl_compute_command_queue);

	clGetEventProfilingInfo(timing_event, CL_PROFILING_COMMAND_START,
		sizeof(time_start), &time_start, NULL);

	clGetEventProfilingInfo(timing_event, CL_PROFILING_COMMAND_END,
		sizeof(time_end), &time_end, NULL);


	read_time = (float)(time_end - time_start) / 1000;
	//printf("Start  = %d sec.\n", time_start);
	//printf("Koniec  = %d sec.\n", time_end);
	//printf("Czas wykonania jadra odpowiedzialnego za skalowanie obrazu  =  %.3f usec. (Mikrosekunda)\n", read_time); // in usec(Mikrosekunda)

	//wyniki_count = sprintf(wyniki_buf, "Czas wykonania jadra odpowiedzialnego za skalowanie z mnożnikiem  %.5f obrazu w mikrosekundach: \n  ", scale_factor);
	//wyniki_count += sprintf(wyniki_buf + wyniki_count, " %.5f \n", read_time);
	//fwrite(wyniki_buf, sizeof(char), strlen(wyniki_buf), wyniki);
	fprintf(wyniki, "Czas wykonania jadra odpowiedzialnego za skalowanie z mnożnikiem  %.5f obrazu w mikrosekundach: \n  ", scale_factor);
	fprintf(wyniki, "%.5f\n", read_time);

	origin[0] = 0;
	origin[1] = 0;
	origin[2] = 0;
	size[0] = scale_w;
	size[1] = scale_h;
	size[2] = 1;

	//output  png

	//create the image with  the same width and height as original
	outPng.Create(scale_w, scale_h);

	//temporary array to store the result from opencl
	tmp = new unsigned char[scale_w * scale_h * 4];
	//CL_TRUE means that it waits for the entire image to be copied before continuing
	cli_err_num = clEnqueueReadImage(cl_compute_command_queue, output_image, CL_TRUE, origin, size, 0, 0, (void*)tmp, 0, NULL, NULL);
	//copy the data from the temp array to the png
	std::copy(&tmp[0], &tmp[scale_w * scale_h * 4], std::back_inserter(outPng.data));

	//write the image to file
	outPng.Save("xx"+p + "_Scale_"+resize +".png");
	//free the iamge's resources since we are done with it
	//outPng.Free();




	/////////////////////////////////////////////////////
	///////drugi kernel skalowannie ////////////////

	cl_mem  input_image2 = clCreateImage2D(cl_compute_context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, &image_format, outPng.w, outPng.h, 0, &outPng.data[0], &cli_err_num);
	cl_mem  output_image2 = clCreateImage2D(cl_compute_context, CL_MEM_WRITE_ONLY, &image_format, outPng.w, outPng.h, 0, NULL, &cli_err_num);

	kernel2 = clCreateKernel(Cl_kernel_program, "blurKernel", &cli_err_num);
	if (cli_err_num != CL_SUCCESS) {
		printf("brak kodu kernela 2\n");
		return 1;
	}
	cli_err_num = clSetKernelArg(kernel2, 0, sizeof(cl_mem), &input_image2);
	cli_err_num = clSetKernelArg(kernel2, 1, sizeof(cl_mem), &output_image2);



	if (cli_err_num != CL_SUCCESS) {
		printf("Nie udało się ustalić parametrów jądraxxx! \n");
		return EXIT_FAILURE;

	}
	else {

		cli_err_num = clSetKernelArg(kernel2, 0, sizeof(cl_mem), &input_image2);
		cli_err_num = clSetKernelArg(kernel2, 1, sizeof(cl_mem), &output_image2);
		if (cli_err_num != CL_SUCCESS) {
			printf("Nie udało się ustalić parametrów jądra! \n");
			return EXIT_FAILURE;
		}
	}


	globalWorkSize2[0] = outPng.w;
	globalWorkSize2[1] = outPng.h;

	cli_err_num = clEnqueueNDRangeKernel(cl_compute_command_queue, kernel2, 2, NULL, globalWorkSize2, NULL, 0, NULL, &timing_event2);
	//wait for kernel to finish
	clFinish(cl_compute_command_queue);

	clGetEventProfilingInfo(timing_event2, CL_PROFILING_COMMAND_START,
		sizeof(time_start2), &time_start2, NULL);

	clGetEventProfilingInfo(timing_event2, CL_PROFILING_COMMAND_END,
		sizeof(time_end2), &time_end2, NULL);


	read_time2 = (float)(time_end2 - time_start2) / 1000;
//	printf("start  = %d sec.\n", time_start2);
//	printf("koniec  = %d sec.\n", time_end2);
//	printf("Czas wykonania jadra odpowiedzialnego za usuwanie szumów =  %.3f usec. (Mikrosekunda)\n", read_time2); // in usec(Mikrosekunda)
	//wyniki_count = sprintf(wyniki_buf, "Czas wykonania jadra odpowiedzialnego za usuwanie szumów  z mnożnikiem  %.5f obrazu w mikrosekundach: \n  ", scale_factor);
	//wyniki_count += sprintf(wyniki_buf + wyniki_count, " %.5f \n", read_time);
	//fwrite(wyniki_buf, sizeof(char), strlen(wyniki_buf), wyniki);
	fprintf(wyniki, "Czas wykonania jadra odpowiedzialnego za usuwanie szumów  z mnożnikiem  %.5f obrazu w mikrosekundach: \n  ", scale_factor);
	fprintf(wyniki, "%.5f\n", read_time2);


	size[0] = outPng.w;
	size[1] = outPng.h;
	//size[2] = 1;


	//create the image with the same width and height as original
	outPng2.Create(outPng.w, outPng.h);

	//temporary array to store the result from opencl
	tmp2 = new unsigned char[outPng.w * outPng.h * 4];
	//CL_TRUE means that it waits for the entire image to be copied before continuing
	cli_err_num = clEnqueueReadImage(cl_compute_command_queue, output_image2, CL_TRUE, origin, size, 0, 0, (void*)tmp2, 0, NULL, NULL);
	//copy the data from the temp array to the png
	std::copy(&tmp2[0], &tmp2[outPng.w * outPng.h * 4], std::back_inserter(outPng2.data));

	//write the image to file
	outPng2.Save("xx"+ p + "_Noise_del_" + resize + ".png");
	outPng2.Free();
	outPng.Free();
	free(tmp);
	free(tmp2);
	
	printf("\n etap end  ");

}
fclose(wyniki);

inPng.Free();
free(opencl_devices);
free(opencl_platforms);
	 return 0;
 }


