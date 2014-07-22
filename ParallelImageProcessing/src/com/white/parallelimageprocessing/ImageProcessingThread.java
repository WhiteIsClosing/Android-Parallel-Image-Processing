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
		// Ҫ�Ђ�if���Д������Ă�native����
		if (command.equals(command_strings[8]))
			/*-------------�õ�GPU��Ϣ--------------*/
			clinfoString = getDeviceInfo();
		else if (command.equals(command_strings[2])) {
			/*---------------SRADȥ��--------------*/
			result_string = SRADdenoising(image_path, itertime);
		} else if (command.equals(command_strings[3])) {
			result_string = sobelfilter(image_path, itertime);
		} else if (command.equals(command_strings[0])) {
			// ��ִ�еĲ���sha1�㷨
			String GPUResult = sha1entryption(image_path, itertime);
			// �������Ǵ��е�sha1�㷨
			MessageDigest md = null;
			byte[] bt = image_path.getBytes();// �õ��ֽ���
			long CPUSHA1StartTime = 0, CPUSHA1EndTime = 0;
			try {
				md = MessageDigest.getInstance("SHA-1");
				// ���е���
				CPUSHA1StartTime = System.currentTimeMillis();
				for (int i = 0; i < itertime; i++) {
					md.update(bt);
					// �õ�cpu���ܵ�����
					/* String CPUCipher = */bytes2Hex(md.digest()); // to
																	// HexString
				}
				CPUSHA1EndTime = System.currentTimeMillis();
			} catch (NoSuchAlgorithmException e) {
				result_string = "No such algorithm!";
				e.printStackTrace();
			}
			// CPUʱ��
			double cpuTimeNum = (CPUSHA1EndTime - CPUSHA1StartTime);
			String tempSTR[] = GPUResult.split("/");
			double SHA1SpeedUp = cpuTimeNum / Double.parseDouble(tempSTR[0]);// ���ٱ�
			result_string = GPUResult + "/" + ((double) cpuTimeNum) / 1000
					+ "/" + ((double) SHA1SpeedUp) / 1000;
		} else if (command.equals(command_strings[5])) {
			/*----------------˫���˲�---------------*/
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
			// ��������������
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
				bmpCPU.compress(Bitmap.CompressFormat.JPEG, 100, out);// 100��ʾ��ѹ��
				out = new FileOutputStream(fgpu);
				bmpGPU.compress(Bitmap.CompressFormat.JPEG, 100, out);// 100��ʾ��ѹ��
				out.flush();
				out.close();
			} catch (FileNotFoundException e) {
				e.printStackTrace();
			} catch (IOException e) {
				e.printStackTrace();
			}
		} else if (command.equals(command_strings[4])) {
			/*---------------��˹ģ��---------------*/
			rsBitmapIn = BitmapFactory.decodeFile(image_path);
			int n = 4;// ��˹ģ������
			// RS��ʱ��ʼ
			long startTime = System.currentTimeMillis();
			for (int i = 0; i < itertime; i++)
				rsBitmapOut = mGBlurPic.gBlurBitmap(rsBitmapIn, n);
			long endTime = System.currentTimeMillis();
			// RS����ʱ��
			long gaussian_filter_time = endTime - startTime;
			/* ��˹�˲���renderscriptʱ�� */
			double GaussianFilterRSTime = ((double) gaussian_filter_time) / 1000;
			// ����RS�ó��ĸ�˹ģ��ͼ��
			File f = new File("/storage/sdcard0/cpu_gaussian.jpg");
			if (f.exists()) {
				f.delete();
			}
			FileOutputStream out;
			try {
				out = new FileOutputStream(f);
				rsBitmapOut.compress(Bitmap.CompressFormat.JPEG, 100, out);// 100��ʾ��ѹ��
				out.flush();
				out.close();
			} catch (FileNotFoundException e) {
				e.printStackTrace();
			} catch (IOException e) {
				e.printStackTrace();
			}
			// OpenCV��ʱ��ʼ
			startTime = System.currentTimeMillis();
			for (int i = 0; i < itertime; i++)
				gaussianBlurFilter(image_path, n);
			endTime = System.currentTimeMillis();
			// OpenCV����ʱ��
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
			// ��ꇳ˷�
			System.out.println("choose matrix");
			result_string = multmatrix(matrix_len, itertime);
		} else {
			// do nothing
		}
	}

	/* ���涼�Ǳ��ط��������� */
	public native String getDeviceInfo();

	// srad���ط���
	public native String SRADdenoising(String image_path, int iter_time);

	// SOBEL�˲����ط���
	public native String sobelfilter(String image_path, int iter_time);

	// sha-1�����㷨
	public native String sha1entryption(String plain_text, int iter_time);

	// sha1�����㷨���в���֮һ
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

	// ˫���˲�
	public native String bilateralfilter(Bitmap bitmapIn, Bitmap bitmapCPU,
			Bitmap bitmapGPU, int width, int height, int itertime);

	// ��ꇳ˷�
	public native String multmatrix(int len, int itertime);

	// ��˹ģ������opencv����
	public native String gaussianBlurFilter(String image_path, int itertime);
}
