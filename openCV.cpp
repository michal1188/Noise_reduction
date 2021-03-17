
#include <iostream>
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"

#include <cstring>

using namespace std;
using namespace cv;

#pragma warning(disable:4996)
int main(int argc, char** argv){

	float Tab_scale_factor[7] = { 0.2, 0.5,0.7,1,2,5,10 };
	int mask_size_tab[5] = { 3,7,15,25,51 };
	float scale_factor;
	//wybierz obraz
	Mat im = imread("balon.png");
	if (im.empty())
	{
		cout << "Cannot open image!" << endl;
		return -1;
	}
	/*
	 float scale;
	 printf("Wpisz wspolczynnik skali:\n");
	 printf("- 1 obraz w rozmiarze orginalu \n");
	 printf("- >1  obraz wiekszy od orginalu \n");
	 printf("- <1  obraz mniejszy od orginalu \n");
	 scanf_s("%f", &scale);
	 fflush(stdin);

	 */
	Mat im_resize;
	Mat im_blur;

	FILE* wyniki;


	char  mask_folder[100];
	char* path_arr;
	string path_arr2;
	  //make the copy 1/2 as big as the original
	float e1, e2, time;

	  for (int i = 0; i < sizeof(mask_size_tab) / sizeof(int); i++) {

		  string path="\\Balon_photo_gaussian_noise\\Sekwencyjny\\";

		  sprintf_s(mask_folder, "%dx%d\\", mask_size_tab[i],  mask_size_tab[i]);
		  path.append(mask_folder);
		  path_arr2 = &path[0];
		  sprintf_s(mask_folder, "wyniki_Balon_photo_gaussian_nois%dx%d.txt", mask_size_tab[i] , mask_size_tab[i]);
		  path.append(mask_folder);
		  path_arr = &path[0];
		  wyniki = fopen(path_arr, "w");
		  if (wyniki == NULL) {
			  perror("fopen failed");
			  exit(1);
		  }
		  fprintf(wyniki, "Maska rozmycia %dx%d\n", mask_size_tab[i],  mask_size_tab[i]);
		  std::printf("\n etap %s  ", path_arr);
		  for (int j = 0; j < sizeof(Tab_scale_factor) / sizeof(float); j++) {
			  e1, e2, time = 0;
			  std::printf("\n etap skalowanie  ");
			  scale_factor = Tab_scale_factor[j];
			  e1 = getTickCount();
			  resize(im, im_resize, Size(im.cols * scale_factor, im.rows * scale_factor), 0, 0, INTER_LINEAR);
			  e2 = getTickCount();
			  time = (e2 - e1) / getTickFrequency();
			  fprintf(wyniki, "Czas wykonania jadra odpowiedzialnego za skalowanie z mno¿nikiem  %.5f obrazu w mikrosekundach: \n  ", scale_factor);
			  fprintf(wyniki, "%.20f\n", time);
			  imwrite(path_arr2+"Balon_Scale_" + to_string(Tab_scale_factor[j]) + ".png", im_resize);

			  e1 = getTickCount();
			  GaussianBlur(im_resize, im_blur, Size(mask_size_tab[i], mask_size_tab[i]), 0, 0);
			  e2 = getTickCount();
			  time = (e2 - e1) / getTickFrequency();
			  fprintf(wyniki, "Czas wykonania jadra odpowiedzialnego za usuwanie szumów  z mno¿nikiem  %.5f obrazu w mikrosekundach: \n  ", scale_factor);
			  fprintf(wyniki, "%.20f\n", time);
			  imwrite(path_arr2 + "Balon_Noise_del_" + to_string(Tab_scale_factor[j]) + ".png", im_blur);
		  }
		  std::fclose(wyniki);
		  std::printf("\n etap end  ");
	  }
/*
	for (int i = 1; i < MAX_KERNEL_LENGTH; i = i + 2)
	{
		medianBlur(src, dst, i);
		if (display_dst(DELAY_BLUR) != 0)
		{
			return 0;
		}
	}

	*/
	return 0;
}

