package edu.sttomas;

import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.sql.Timestamp;
import java.util.Arrays;
import java.util.Comparator;
import java.util.Date;
import java.util.List;
import java.util.UUID;
import java.util.stream.Collectors;

import javax.servlet.http.HttpServletResponse;

import org.apache.commons.io.FileUtils;
import org.apache.commons.io.IOUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Controller;
import org.springframework.ui.Model;
import org.springframework.util.FileCopyUtils;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestMethod;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.multipart.MultipartFile;
import org.springframework.web.servlet.mvc.support.RedirectAttributes;

@Controller
public class PhotoController {
	
	@Autowired
	private MailBoxImageCRUD mailBoxImageCRUD;
	
	@RequestMapping(method = RequestMethod.GET, value = "/cameras/img")
	public String provideUploadInfo(Model model) {
		File rootFolder = new File(Application.ROOT);
		@SuppressWarnings("unused")
		List<String> fileNames = Arrays.stream(rootFolder.listFiles()).map(f -> f.getName())
				.collect(Collectors.toList());

		model.addAttribute("files",
				Arrays.stream(rootFolder.listFiles()).sorted(Comparator.comparingLong(f -> -1 * f.lastModified()))
						.map(f -> f.getName()).collect(Collectors.toList()));

		return "uploadForm";
	}

	@RequestMapping(method = RequestMethod.POST, value = "/cameras/img")
	public String handleFileUpload(@RequestParam("USER_ID") String userID, @RequestParam("DEVICE_ID") String deviceID,
			@RequestParam("status") int status, @RequestParam("file") MultipartFile file,
			RedirectAttributes redirectAttributes) {

		String fileName = UUID.randomUUID().toString().replaceAll("-", "") + ".jpg";
		
		if (!file.isEmpty()) {
			try {
				BufferedOutputStream stream = new BufferedOutputStream(
						new FileOutputStream(new File(Application.ROOT + "/" + fileName)));
				FileCopyUtils.copy(file.getInputStream(), stream);
				stream.close();
				redirectAttributes.addFlashAttribute("message", "You successfully uploaded " + fileName + "!");
			
				
			} catch (Exception e) {
				redirectAttributes.addFlashAttribute("message",
						"You failed to upload " + userID + " => " + e.getMessage());
			}
		} else {
			redirectAttributes.addFlashAttribute("message",
					"You failed to upload " + userID + " because the file was empty");
		}
		
		MailBoxImage mailBoxImage = new MailBoxImage(UUID.randomUUID().toString().replaceAll("-", ""), userID, deviceID, status, fileName, new Timestamp(new Date().getTime()));
		mailBoxImageCRUD.save(mailBoxImage);

		return "redirect:/cameras/img";
	}

	@RequestMapping(value = "/files/{file_name}", method = RequestMethod.GET)
	public void getFile(@PathVariable("file_name") String fileName, HttpServletResponse response) {
		try {
			// get your file as InputStream
			InputStream inputStream = FileUtils.openInputStream(new File(Application.ROOT + "/" + fileName));
			IOUtils.copy(inputStream,
					response.getOutputStream());
			inputStream.close();
			response.flushBuffer();
		} catch (IOException ex) {
			throw new RuntimeException("IOError writing file to output stream");
		}
	}
}
