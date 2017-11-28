#include <fltKernel.h>
#include <dontuse.h>
#include <suppress.h>

PFLT_FILTER MiniFilterHandle = NULL;
FLT_PREOP_CALLBACK_STATUS MiniPreCreate(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS FltObjects, PVOID *CompletionContex);
FLT_POSTOP_CALLBACK_STATUS MiniPostCreate(FLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS FltObjects, PVOID *CompletionContex);
FLT_PREOP_CALLBACK_STATUS MiniPreWrite(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS FltObjects, PVOID* CompletionContex);
NTSTATUS MiniUnload(FLT_FILTER_UNLOAD_FLAGS Flags);


FLT_PREOP_CALLBACK_STATUS MiniPreCreate(PFLT_CALLBACK_DATA Data,
	PCFLT_RELATED_OBJECTS FltObjects,
	PVOID *CompletionContex) {
	PFLT_FILE_NAME_INFORMATION FileNameInfor;
	NTSTATUS status = STATUS_SUCCESS;

	status = FltGetFileNameInformation(Data, FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT, &FileNameInfor);
	status = FltParseFileNameInformation(FileNameInfor);

	/// FileNameInfor->Name.Buffer // se in ra toan bo o dia
	// Them chuc nang dua ra duong link tren user

	PUNICODE_STRING pFullPath;
	UNICODE_STRING volumeName;

	volumeName.Buffer = NULL;

	// khoi tao chuoi
	RtlInitUnicodeString(pFullPath, NULL);

	// thuực hiện chức năng nối chuổi 
	if (NT_SUCCESS(status)) {
		PDEVICE_OBJECT pDeviceDisk = NULL;
		status = FltGetDiskDeviceObject(FltObjects->Volume, &pDeviceDisk);
		if (NT_SUCCESS(status)) {
			status = IoVolumeDeviceToDosName(pDeviceDisk, &volumeName);
			if (NT_SUCCESS(status)) {
				// thuc hien ghep chuoi
				pFullPath->MaximumLength = volumeName.Length + FileNameInfor->Name.Length + sizeof(UNICODE_NULL);
				pFullPath->Buffer = ExAllocatePoolWithTag(NonPagedPool, pFullPath->MaximumLength, 'KHAI');
				status = RtlAppendUnicodeStringToString(pFullPath, &volumeName);
				if (NT_SUCCESS(status)) {
					status = RtlAppendUnicodeStringToString(pFullPath, &FileNameInfor->ParentDir);
					if (NT_SUCCESS(status)) {
						status = RtlAppendUnicodeStringToString(pFullPath, &FileNameInfor->Name);
						DbgPrint("Duong dan file : %ws\n", pFullPath->Buffer);

					}

				}

				ExFreePoolWithTag(pFullPath->Buffer, 'KHAI');
			}
		}
		ObDereferenceObject(pDeviceDisk);//giai phong tham  chieu toi diskdevice
	}

	// querry 
	// , giai phong cau truc P_FLT_FILE_NAME_INFORMATION, giai phong vung nho da cap
	
	FltReleaseFileNameInformation(FileNameInfor);
	return FLT_PREOP_SUCCESS_WITH_CALLBACK;

}

FLT_POSTOP_CALLBACK_STATUS MiniPostCreate(FLT_CALLBACK_DATA Data, 
											PCFLT_RELATED_OBJECTS FltObjects,
											PVOID *CompletionContex) {
	DbgPrint("Test MiniPostCreate Callback!!\n");
	return FLT_POSTOP_FINISHED_PROCESSING;

}

FLT_PREOP_CALLBACK_STATUS MiniPreWrite(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS FltObjects, PVOID* CompletionContex) {
	PFLT_FILE_NAME_INFORMATION FileNameInfor;
	NTSTATUS status;
	WCHAR Extension[10] = { 0 }; // try block .doc Extension;

	status = FltGetFileNameInformation(Data, FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT, &FileNameInfor);
	status = FltParseFileNameInformation(FileNameInfor);
	if (NT_SUCCESS(status)) {
		RtlCopyMemory(Extension, FileNameInfor->Extension.Buffer, FileNameInfor->Extension.MaximumLength);// bat tat ca doc*;
		if (wcsstr(Extension, L"doc") != NULL) {
			DbgPrint("File name %ws bi block!\n", FileNameInfor->Name.Buffer);
			Data->IoStatus.Status = STATUS_INVALID_PARAMETER;
			Data->IoStatus.Information = 0; // con tro toi thong tin vung loi
			FltReleaseFileNameInformation(FileNameInfor);

			return FLT_PREOP_COMPLETE;
		}
	}
	FltReleaseFileNameInformation(FileNameInfor);
	return FLT_PREOP_SUCCESS_WITH_CALLBACK;
}

NTSTATUS MiniUnload(FLT_FILTER_UNLOAD_FLAGS Flags) {
	DbgPrint("Driver Unloaded !! \n");

	return STATUS_SUCCESS;

}
const FLT_OPERATION_REGISTRATION Callbacks[] = { { IRP_MJ_CREATE,
													0,
													MiniPreCreate,
													MiniPostCreate },

													{ IRP_MJ_WRITE,
													0,
													MiniPreWrite,
													NULL },
													{ IRP_MJ_OPERATION_END }

													};
const FLT_REGISTRATION FilterRegistration = { sizeof(FLT_REGISTRATION),
												FLT_REGISTRATION_VERSION,
												NULL,
												NULL,
												Callbacks,
												MiniUnload,
												NULL,
												NULL,
												NULL,
												NULL,
												NULL,
												NULL,
												NULL,
												NULL };

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath) {
	NTSTATUS status = STATUS_SUCCESS;
	status = FltRegisterFilter(DriverObject, &FilterRegistration, &MiniFilterHandle);
	if (NT_SUCCESS(status)) {
		FltStartFiltering(MiniFilterHandle);
		if (!NT_SUCCESS(status)) {
			DbgPrint(" MiniFilter canot start !!\n");
			FltUnregisterFilter(MiniFilterHandle);
		}

	}

	return status;
}