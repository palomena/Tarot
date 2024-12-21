int main() {
	float width = 80.0f;
	float height = 40.0f;
	float max = 10.0f;
	float row = 0.00f;
	while (row < height) {
		float col = 0.00f;
		while (col < width) {
			float c_re = (col - width/2.00f)*4.00f/width;
			float c_im = (row - height/2.00f)*4.00f/width;
			float x = 0.00f;
			float y = 0.00f;
			float iteration = 0.00f;
			while (x*x+y*y <= 4.0f && iteration < max) {
				float x_new = x*x - y*y + c_re;
				y = 2.00f*x*y + c_im;
				x = x_new;
				iteration = iteration + 1.00f;
			}
			if (iteration < max) {
				printf(" ");
			} else {
				printf("x");
			}
			col = col + 1.00f;
		}
		row = row + 1.00f;
		printf("\n");
	}
	return 0;
}
