package com.white.parallelimageprocessing;

import android.app.Activity;
import android.content.Intent;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.net.Uri;
import android.os.Bundle;
import android.provider.MediaStore;
import android.view.View;
import android.view.inputmethod.InputMethodManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

public class ImageProcessingActivity extends Activity {
	private Intent fromMain;// intent from MainActivity
	private String command_stirng;
	private String imagePathString, resultString;
	private ImageView original_image, cpu_image, gpu_image;
	private TextView original_textview, cpu_textview, gpu_textview;
	private Button choose_button, processing_button, results_button;
	private EditText iter_edittext;
	private int iterTime = 1;
	private int RESULT_LOAD_IMAGE = 3;
	private Button.OnClickListener listener;
	private ImageProcessingThread ipt;

	private Bitmap handleOOM(String absolutePath, ImageView imageView) {
		Bitmap bm;
		BitmapFactory.Options opt = new BitmapFactory.Options();
		// 这个isjustdecodebounds很重要
		opt.inJustDecodeBounds = true;
		bm = BitmapFactory.decodeFile(absolutePath, opt);

		// 获取到这个图片的原始宽度和高度
		int picWidth = opt.outWidth;
		int picHeight = opt.outHeight;

		int width = imageView.getWidth();
		int height = imageView.getHeight();
		// isSampleSize是表示对图片的缩放程度，比如值为2图片的宽度和高度都变为以前的1/2
		opt.inSampleSize = 1;

		// 根据imageview的大小和图片大小计算出缩放比例
		if (picWidth > picHeight) {
			if (picWidth > width)
				opt.inSampleSize = picWidth / width;
		} else {
			if (picHeight > height)
				opt.inSampleSize = picHeight / height;
		}
		// 这次再真正地生成一个有像素的，经过缩放了的bitmap
		opt.inJustDecodeBounds = false;
		bm = BitmapFactory.decodeFile(absolutePath, opt);
		return bm;
	}

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		fromMain = this.getIntent();
		command_stirng = fromMain.getStringExtra("command");
		setContentView(R.layout.activity_image_processing);
		initView();
	}

	private void initView() {
		iter_edittext = (EditText) findViewById(R.id.iter_edittext);
		choose_button = (Button) findViewById(R.id.choose_button);
		processing_button = (Button) findViewById(R.id.processing_button);
		results_button = (Button) findViewById(R.id.result_button);
		original_textview = (TextView) findViewById(R.id.original_textview);
		cpu_textview = (TextView) findViewById(R.id.cpu_text);
		gpu_textview = (TextView) findViewById(R.id.gpu_text);
		original_image = (ImageView) findViewById(R.id.original_image);
		cpu_image = (ImageView) findViewById(R.id.cpu_image);
		gpu_image = (ImageView) findViewById(R.id.gpu_image);
		processing_button.setEnabled(false);
		results_button.setEnabled(false);
		cpu_textview.setVisibility(View.INVISIBLE);
		gpu_textview.setVisibility(View.INVISIBLE);
		original_textview.setVisibility(View.INVISIBLE);
		original_image.setVisibility(View.INVISIBLE);
		cpu_image.setVisibility(View.INVISIBLE);
		gpu_image.setVisibility(View.INVISIBLE);
		listener = new Button.OnClickListener() {
			public void onClick(View v) {
				switch (v.getId()) {
				case R.id.choose_button:
					Intent i = new Intent(
							Intent.ACTION_PICK,
							android.provider.MediaStore.Images.Media.EXTERNAL_CONTENT_URI);
					cpu_textview.setVisibility(View.INVISIBLE);
					gpu_textview.setVisibility(View.INVISIBLE);
					cpu_image.setVisibility(View.INVISIBLE);
					gpu_image.setVisibility(View.INVISIBLE);
					((InputMethodManager) getSystemService(INPUT_METHOD_SERVICE))
							.hideSoftInputFromWindow(
									ImageProcessingActivity.this
											.getCurrentFocus().getWindowToken(),
									InputMethodManager.HIDE_NOT_ALWAYS);
					startActivityForResult(i, RESULT_LOAD_IMAGE);
					break;
				case R.id.processing_button:
					String iter_string = iter_edittext.getText().toString();
					if (!iter_string.equals("")) {
						iterTime = Integer.parseInt(iter_string);
						Toast.makeText(ImageProcessingActivity.this,
								"Start processing:" + imagePathString,
								Toast.LENGTH_SHORT).show();
						results_button.setEnabled(true);
						int command_index = 0;
						if (command_stirng.equals(getResources()
								.getStringArray(R.array.command)[2])) {
							command_index = 2;
						} else if (command_stirng.equals(getResources()
								.getStringArray(R.array.command)[3])) {
							command_index = 3;
						} else if (command_stirng.equals(getResources()
								.getStringArray(R.array.command)[5])) {
							command_index = 5;// 双边滤波
						} else if (command_stirng.equals(getResources()
								.getStringArray(R.array.command)[4])) {
							command_index = 4;// 双边滤波
						}
						// 选择不同的算法的时候 给command_index赋值
						ipt = new ImageProcessingThread(
								ImageProcessingActivity.this,
								ImageProcessingActivity.this.getResources()
										.getStringArray(R.array.command)[command_index],
								imagePathString, iterTime);
						ipt.start();

					} else {
						Toast.makeText(
								ImageProcessingActivity.this,
								ImageProcessingActivity.this.getResources()
										.getString(R.string.please_input_iter),
								Toast.LENGTH_LONG).show();
					}
					break;
				case R.id.result_button:
					resultString = ipt.result_string;
					if (resultString
							.equals(getApplicationContext().getResources()
									.getString(R.string.processing_wait))) {
						// do nothing
					} else {
						// 下面显示结果
						cpu_textview.setVisibility(View.VISIBLE);
						gpu_textview.setVisibility(View.VISIBLE);
						gpu_image.setVisibility(View.VISIBLE);
						cpu_image.setVisibility(View.VISIBLE);
						// 下面这里显示结果的路径需要一个选择结构
						if (command_stirng.equals(getResources()
								.getStringArray(R.array.command)[2])) {
							cpu_image.setImageBitmap(handleOOM(
									"/storage/sdcard0/srad_cpu_result.jpg",
									cpu_image));
							gpu_image.setImageBitmap(handleOOM(
									"/storage/sdcard0/srad_gpu_result.jpg",
									gpu_image));
						} else if (command_stirng.equals(getResources()
								.getStringArray(R.array.command)[3])) {
							cpu_image
									.setImageBitmap(handleOOM(
											"/storage/sdcard0/cpu_sobel.jpg",
											cpu_image));
							gpu_image
									.setImageBitmap(handleOOM(
											"/storage/sdcard0/gpu_sobel.jpg",
											gpu_image));
						} else if (command_stirng.equals(getResources()
								.getStringArray(R.array.command)[5])) {
							// 双边滤波
							if (!resultString.equals(getApplicationContext()
									.getResources().getString(
											R.string.processing_wait))) {
								cpu_image.setImageBitmap(ipt.bmpCPU);
								gpu_image.setImageBitmap(ipt.bmpGPU);
							}

						} else if (command_stirng.equals(getResources()
								.getStringArray(R.array.command)[4])) {
							// RenderScript在GPU上的处理结果
							gpu_image.setImageBitmap(ipt.rsBitmapOut);
							// 利用OpenCV在CPU上的处理结果
							cpu_image.setImageBitmap(handleOOM(
									"/storage/sdcard0/cpu_gaussian.jpg",
									cpu_image));
						}
					}

					Toast.makeText(getApplicationContext(), resultString,
							Toast.LENGTH_LONG).show();
					break;
				default:
					break;
				}
			}
		};
		choose_button.setOnClickListener(listener);
		processing_button.setOnClickListener(listener);
		results_button.setOnClickListener(listener);
	}

	@Override
	protected void onStart() {
		super.onStart();
		Toast.makeText(this, command_stirng, Toast.LENGTH_SHORT).show();
	}

	protected void onActivityResult(int requestCode, int resultCode, Intent data) {
		super.onActivityResult(requestCode, resultCode, data);

		if (requestCode == RESULT_LOAD_IMAGE && resultCode == RESULT_OK
				&& null != data) {
			Uri selectedImage = data.getData();
			String[] filePathColumn = { MediaStore.Images.Media.DATA };
			Cursor cursor = getContentResolver().query(selectedImage,
					filePathColumn, null, null, null);
			cursor.moveToFirst();
			int columnIndex = cursor.getColumnIndex(filePathColumn[0]);
			imagePathString = cursor.getString(columnIndex);
			cursor.close();
			original_textview.setVisibility(View.VISIBLE);
			original_image.setVisibility(View.VISIBLE);
			processing_button.setEnabled(true);
			original_image.setImageBitmap(handleOOM(imagePathString,
					original_image));
		}
	}
}
