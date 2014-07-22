package com.white.parallelimageprocessing;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Bitmap.Config;

public class ImageProcessingThread extends Thread {
	static {
		System.loadLibrary("parallel_image_processing");
	}
	public String clinfoString;
	private Context ct;
	private String[] command_strings;
	private String command;
	private int itertime;
	private String image_path;
	public String result_string;
	Bitmap bmpOrig, bmpCPU, bmpGPU;
	Bitmap rsBitmapIn, rsBitmapOut;
	private GBlurPic mGBlurPic;
	public int matrix_len = 16;

	public ImageProcessingThread() {
	}

	public ImageProcessingThread(Context ct, String command, int itertime,
			int len) {
		this.matrix_len = len;
		this.ct = ct;
		this.command = command;
		this.itertime = itertime;
		command_strings = ct.getResources().getStringArray(R.array.command);
		result_string = ct.getResources().getString(R.string.processing_wait);
	}

	public ImageProcessingThread(Context ct, String command, String image_path,
			int itertime) {
		this.image_path = image_path;
		this.itertime = itertime;
		this.command = command;
		this.ct = ct;
		clinfoString = ct.getResources().getString(R.string.getting_info);
		command_strings = ct.getResources().getStringArray(R.array.command);
		result_string = ct.getResources().getString(R.string.processing_wait);
		mGBlurPic = new GBlurPic(ct);
	}

	public void run() {
		// 要有個if來判斷執行哪個native方法
		if (command.equals(command_strings[8]))
			/*-------------得到GPU信息--------------*/
			clinfoString = getDeviceInfo();
		else if (command.equals(command_strings[2])) {
			/*---------------SRAD去噪--------------*/
			result_string = SRADdenoising(image_path, itertime);
		} else if (command.equals(command_strings[3])) {
			result_string = sobelfilter(image_path, itertime);
		} else if (command.equals(command_strings[0])) {
			// 先执行的并行sha1算法
			String GPUResult = sha1entryption(image_path, itertime);
			// 接下来是串行的sha1算法
			MessageDigest md = null;
			byte[] bt = image_path.getBytes();// 得到字节流
			long CPUSHA1StartTime = 0, CPUSHA1EndTime = 0;
			try {
				md = MessageDigest.getInstance("SHA-1");
				// 串行叠代
				CPUSHA1StartTime = System.currentTimeMillis();
				for (int i = 0; i < itertime; i++) {
					md.update(bt);
					// 得到cpu加密的密文
					/* String CPUCipher = */bytes2Hex(md.digest()); // to
																	// HexString
				}
				CPUSHA1EndTime = System.currentTimeMillis();
			} catch (NoSuchAlgorithmException e) {
				result_string = "No such algorithm!";
				e.printStackTrace();
			}
			// CPU时间
			double cpuTimeNum = (CPUSHA1EndTime - CPUSHA1StartTime);
			String tempSTR[] = GPUResult.split("/");
			double SHA1SpeedUp = cpuTimeNum / Double.parseDouble(tempSTR[0]);// 加速比
			result_string = GPUResult + "/" + ((double) cpuTimeNum) / 1000
					+ "/" + ((double) SHA1SpeedUp) / 1000;
		} else if (command.equals(command_strings[5])) {
			/*----------------双边滤波---------------*/
			Bitmap bmpOrig = BitmapFactory.decodeFile(image_path);
			bmpCPU = Bitmap.createBitmap(bmpOrig.getWidth(),
					bmpOrig.getHeight(), Config.ARGB_8888);
			bmpGPU = Bitmap.createBitmap(bmpOrig.getWidth(),
					bmpOrig.getHeight(), Config.ARGB_8888);
			result_string = bilateralfilter(bmpOrig, bmpCPU, bmpGPU,
					bmpOrig.getWidth(), bmpOrig.getHeight(), itertime);
			// System.out.println("result" + result_string);
			String[] temp_result = result_string.split("/");
			double bilateralCPU = (double) Integer.parseInt(temp_result[0]);
			double bilateralGPU = (double) Integer.parseInt(temp_result[1]);
			// 下面这三个变量
			double bilateralSpeedUP = (double) bilateralCPU / bilateralGPU;
			double bilateralCPUTime = bilateralCPU / 1000;
			double bilateralGPUTime = bilateralGPU / 1000;
			result_string = "Bilateral result:\nGPU time:" + bilateralGPUTime
					+ "s CPU time:" + bilateralCPUTime + "s\nSpeed up:"
					+ bilateralSpeedUP;
			File fcpu = new File("/storage/sdcard0/cpu_bilateral.jpg");
			File fgpu = new File("/storage/sdcard0/gpu_bilateral.jpg");
			if (fcpu.exists()) {
				fcpu.delete();
			}
			if (fgpu.exists()) {
				fgpu.delete();
			}
			FileOutputStream out;
			try {
				out = new FileOutputStream(fcpu);
				bmpCPU.compress(Bitmap.CompressFormat.JPEG, 100, out);// 100表示不压缩
				out = new FileOutputStream(fgpu);
				bmpGPU.compress(Bitmap.CompressFormat.JPEG, 100, out);// 100表示不压缩
				out.flush();
				out.close();
			} catch (FileNotFoundException e) {
				e.printStackTrace();
			} catch (IOException e) {
				e.printStackTrace();
			}
		} else if (command.equals(command_strings[4])) {
			/*---------------高斯模糊---------------*/
			rsBitmapIn = BitmapFactory.decodeFile(image_path);
			int n = 4;// 高斯模糊参数
			// RS计时开始
			long startTime = System.currentTimeMillis();
			for (int i = 0; i < itertime; i++)
				rsBitmapOut = mGBlurPic.gBlurBitmap(rsBitmapIn, n);
			long endTime = System.currentTimeMillis();
			// RS运行时间
			long gaussian_filter_time = endTime - startTime;
			/* 高斯滤波的renderscript时间 */
			double GaussianFilterRSTime = ((double) gaussian_filter_time) / 1000;
			// 保存RS得出的高斯模糊图像
			File f = new File("/storage/sdcard0/cpu_gaussian.jpg");
			if (f.exists()) {
				f.delete();
			}
			FileOutputStream out;
			try {
				out = new FileOutputStream(f);
				rsBitmapOut.compress(Bitmap.CompressFormat.JPEG, 100, out);// 100表示不压缩
				out.flush();
				out.close();
			} catch (FileNotFoundException e) {
				e.printStackTrace();
			} catch (IOException e) {
				e.printStackTrace();
			}
			// OpenCV计时开始
			startTime = System.currentTimeMillis();
			for (int i = 0; i < itertime; i++)
				gaussianBlurFilter(image_path, n);
			endTime = System.currentTimeMillis();
			// OpenCV运行时间
			long serial_gaussian_filter_time = endTime - startTime;
			double GaussianFilterCPUTime = ((double) serial_gaussian_filter_time) / 1000;

			result_string = ct.getResources().getString(
					R.string.Gaussian_filter_results)
					+ "\nRenderScript time:"
					+ GaussianFilterRSTime
					+ "s\nCPU time:"
					+ GaussianFilterCPUTime
					+ "\nSpeed Up:"
					+ GaussianFilterCPUTime / GaussianFilterRSTime;
		} else if (command.equals(command_strings[7])) {
			// 矩陣乘法
			System.out.println("choose matrix");
			result_string = multmatrix(matrix_len, itertime);
		} else {
			// do nothing
		}
	}

	/* 下面都是本地方法的声明 */
	public native String getDeviceInfo();

	// srad本地方法
	public native String SRADdenoising(String image_path, int iter_time);

	// SOBEL滤波本地方法
	public native String sobelfilter(String image_path, int iter_time);

	// sha-1加密算法
	public native String sha1entryption(String plain_text, int iter_time);

	// sha1加密算法串行步骤之一
	public String bytes2Hex(byte[] bts) {
		String des = "";
		String tmp = null;
		for (int i = 0; i < bts.length; i++) {
			tmp = (Integer.toHexString(bts[i] & 0xFF));
			if (tmp.length() == 1) {
				des = "0";
			}
			des = tmp;
		}
		return des;
	}

	// 双边滤波
	public native String bilateralfilter(Bitmap bitmapIn, Bitmap bitmapCPU,
			Bitmap bitmapGPU, int width, int height, int itertime);

	// 矩陣乘法
	public native String multmatrix(int len, int itertime);

	// 高斯模糊串行opencv程序
	public native String gaussianBlurFilter(String image_path, int itertime);
}
