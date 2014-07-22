package com.white.parallelimageprocessing;

import org.opencv.android.BaseLoaderCallback;
import org.opencv.android.LoaderCallbackInterface;
import org.opencv.android.OpenCVLoader;
import android.os.Bundle;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.Intent;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;

public class MainActivity extends Activity {
	private View sha_1_view, nlmeans_view, k_means_view, sobel_view,
			gaussian_view, bilateral_view, SRAD_view, clinfo_view, matrix_view;
	private View.OnClickListener listener;
	private View.OnTouchListener touch_listener;
	private String[] command_strings;
	private BaseLoaderCallback mLoaderCallback = new BaseLoaderCallback(this) {
		@Override
		public void onManagerConnected(int status) {
			switch (status) {
			case LoaderCallbackInterface.SUCCESS: {
			}
				break;
			default: {
				super.onManagerConnected(status);
			}
				break;
			}
		}
	};

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		OpenCVLoader.initAsync(OpenCVLoader.OPENCV_VERSION_2_4_8, this,
				mLoaderCallback);// 引用opencv manager
		setContentView(R.layout.activity_main);
		initView();
		command_strings = getResources().getStringArray(R.array.command);
	}

	private void initView() {
		sha_1_view = (View) findViewById(R.id.sha_1_view);
		nlmeans_view = (View) findViewById(R.id.nlmeans_view);
		k_means_view = (View) findViewById(R.id.k_means_view);
		sobel_view = (View) findViewById(R.id.sobel_view);
		gaussian_view = (View) findViewById(R.id.gaussian_view);
		bilateral_view = (View) findViewById(R.id.bilateral_view);
		SRAD_view = (View) findViewById(R.id.SRAD_view);
		clinfo_view = (View) findViewById(R.id.cl_device_info);
		matrix_view = (View) findViewById(R.id.matrix_multiplication);
		touch_listener = new View.OnTouchListener() {
			@Override
			public boolean onTouch(View v, MotionEvent event) {
				if (event.getAction() == MotionEvent.ACTION_DOWN)
					v.setAlpha((float) 0.5);
				else {
					v.setAlpha((float) 1.0);
				}
				return false;
			}

		};
		listener = new View.OnClickListener() {
			public void onClick(View v) {
				String image_command = null;
				Intent toNext = new Intent();
				// 下面这个switch决定跳转的activity
				Class<?> cls = null;
				switch (v.getId()) {
				case R.id.sha_1_view:
					image_command = command_strings[0];
					// cls;这里给cls赋值
					cls = SHA1EntryptionActivity.class;
					break;
				case R.id.nlmeans_view:
					image_command = command_strings[1];
					cls = ImageProcessingActivity.class;
					break;
				case R.id.SRAD_view:
					image_command = command_strings[2];
					cls = ImageProcessingActivity.class;
					break;
				case R.id.sobel_view:
					image_command = command_strings[3];
					cls = ImageProcessingActivity.class;
					break;
				case R.id.gaussian_view:
					image_command = command_strings[4];
					cls = ImageProcessingActivity.class;
					break;
				case R.id.bilateral_view:// 双边滤波
					image_command = command_strings[5];
					cls = ImageProcessingActivity.class;
					break;
				case R.id.k_means_view:
					image_command = command_strings[6];
					cls = ImageProcessingActivity.class;
					break;
				case R.id.matrix_multiplication:
					image_command = command_strings[7];
					cls = MatrixActivity.class;
					break;
				case R.id.cl_device_info:
					image_command = command_strings[8];
					cls = ClInfoActivity.class;
					break;
				default:
					break;
				}
				toNext.setClass(MainActivity.this, cls);
				toNext.putExtra("command", image_command);
				startActivity(toNext);
			}
		};
		nlmeans_view.setOnClickListener(listener);
		sha_1_view.setOnClickListener(listener);
		k_means_view.setOnClickListener(listener);
		sobel_view.setOnClickListener(listener);
		bilateral_view.setOnClickListener(listener);
		gaussian_view.setOnClickListener(listener);
		SRAD_view.setOnClickListener(listener);
		clinfo_view.setOnClickListener(listener);
		matrix_view.setOnClickListener(listener);

		nlmeans_view.setOnTouchListener(touch_listener);
		sha_1_view.setOnTouchListener(touch_listener);
		k_means_view.setOnTouchListener(touch_listener);
		sobel_view.setOnTouchListener(touch_listener);
		bilateral_view.setOnTouchListener(touch_listener);
		gaussian_view.setOnTouchListener(touch_listener);
		SRAD_view.setOnTouchListener(touch_listener);
		clinfo_view.setOnTouchListener(touch_listener);
		matrix_view.setOnTouchListener(touch_listener);
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		getMenuInflater().inflate(R.menu.main, menu);
		return true;
	}

	public boolean onOptionsItemSelected(MenuItem item) {
		if (item.getItemId() == R.id.about_menu) {
			Dialog about_dialog = new AlertDialog.Builder(this)
					.setTitle("About this program")
					.setMessage(
							"This program demonstrates some parallel computing instance on android.\nCode by white from XDU.\nwhiteIsClose@gmail.com")
					.create();
			about_dialog.show();
		}

		return true;
	}
}
