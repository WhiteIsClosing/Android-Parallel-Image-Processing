package com.white.parallelimageprocessing;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import android.content.Context;
import android.content.res.AssetManager;

public class ReleaseKernelFile {
	private Context ct;
	private AssetManager assetManager;
	private String[] kernelFileNames;
	private String kernelFileFolder, kernelReleasePath;// cl文件目录
	private File clFileRelease;

	public ReleaseKernelFile(Context ct) {
		this.ct = ct;
		assetManager = ct.getAssets();
		kernelFileFolder = ct.getResources().getString(
				R.string.kernel_folder_name);
		kernelReleasePath = ct.getResources().getString(
				R.string.kernel_release_path);
	}

	public boolean releaseFile() {
		try {
			kernelFileNames = this.ct.getResources().getAssets()
					.list(kernelFileFolder);
		} catch (IOException e) {
			e.printStackTrace();
		}
		clFileRelease = new File(kernelReleasePath);
		// if this directory does not exists, make one.
		if (!clFileRelease.exists()) {
			if (!clFileRelease.mkdirs()) {
				return false;
			}
		}
		// 遍历assets文件夹
		for (int i = 0; i < kernelFileNames.length; i++) {
			String tempFileName = kernelFileNames[i];
			File tempOutFile = new File(kernelReleasePath, tempFileName);
			if (tempOutFile.exists())
				tempOutFile.delete();
			InputStream in = null;
			OutputStream out = null;
			try {
				in = ct.getAssets().open(kernelFileFolder + "/" + tempFileName);
				out = new FileOutputStream(tempOutFile);

				// Transfer bytes from in to out
				byte[] buf = new byte[1024];
				int len;
				while ((len = in.read(buf)) > 0) {
					out.write(buf, 0, len);
				}
				in.close();
				out.close();

			} catch (IOException e) {
				e.printStackTrace();
			}

		}
		return false;
	}
}
