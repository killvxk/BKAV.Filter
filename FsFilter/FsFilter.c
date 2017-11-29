#include <fltKernel.h>
#include <dontuse.h>
#include <suppress.h>

PFLT_FILTER MiniFilterHandle = NULL;
FLT_PREOP_CALLBACK_STATUS MiniPreCreate(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS FltObjects, PVOID *CompletionContex);
FLT_POSTOP_CALLBACK_STATUS MiniPostCreate(FLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS FltObjects, PVOID *CompletionContex, FLT_POST_OPERATION_FLAGS Flags);
FLT_PREOP_CALLBACK_STATUS MiniPreWrite(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS FltObjects, PVOID* CompletionContex);
NTSTATUS MiniUnload(FLT_FILTER_UNLOAD_FLAGS Flags);


FLT_PREOP_CALLBACK_STATUS MiniPreCreate(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS FltObjects, PVOID *CompletionContex) {
	PFLT_FILE_NAME_INFORMATION FileNameInfor;
	NTSTATUS status;
	WCHAR *Name;

	if (FltObjects->FileObject != NULL) {
		status = FltGetFileNameInformation(Data, FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT, &FileNameInfor);
		if (NT_SUCCESS(status)) {
			DbgPrint("print before FltParseFileNameInformation: %wz \r\n", FileNameInfor->Name.Buffer);
			status = FltParseFileNameInformation(FileNameInfor);
			DbgPrint("Test for FileNameInfor->Nam : %wZ \r\n", FileNameInfor->NamesParsed);

			if (NT_SUCCESS(status)) {
				Name = (WCHAR*)ExAllocatePoolWithTag(NonPagedPool, FileNameInfor->Name.MaximumLength, '002A');
				RtlCopyMemory(Name, FileNameInfor->Name.Buffer, FileNameInfor->Name.MaximumLength);
				DbgPrint("Create file : %wz\n", Name);
				ExFreePool(Name);
			}
		}
		FltReleaseFileNameInformation(FileNameInfor);
	}
	
	return FLT_PREOP_SUCCESS_WITH_CALLBACK;

}

FLT_POSTOP_CALLBACK_STATUS MiniPostCreate(FLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS FltObjects,
											PVOID *CompletionContex, FLT_POST_OPERATION_FLAGS Flags) {
	DbgPrint("Test MiniPostCreate Callback!!\n");
	return FLT_POSTOP_FINISHED_PROCESSING;

}

FLT_PREOP_CALLBACK_STATUS MiniPreWrite(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS FltObjects, PVOID* CompletionContex) {
	PFLT_FILE_NAME_INFORMATION FileNameInfor;
	NTSTATUS status;
	WCHAR Name[100] = { 0 }; // try block .doc Extension;

	if (FltObjects->FileObject != NULL) {
		status = FltGetFileNameInformation(Data, FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT, &FileNameInfor);
		if (NT_SUCCESS(status)) {
			status = FltParseFileNameInformation(FileNameInfor);
			if (NT_SUCCESS(status)) {
				RtlCopyMemory(Name, FileNameInfor->Name.Buffer, FileNameInfor->Name.MaximumLength);// bat tat ca doc*;
				_wcsupr(Name);
				if (wcsstr(Name, L"KHAI.TXT") != NULL) {
					DbgPrint("File name %ws bi block!\n", Name);
					Data->IoStatus.Status = STATUS_INVALID_PARAMETER;
					Data->IoStatus.Information = 0; // con tro toi thong tin vung loi
					FltReleaseFileNameInformation(FileNameInfor);
					return FLT_PREOP_COMPLETE;
				}
			}
			FltReleaseFileNameInformation(FileNameInfor);
		}
	}
	
	return FLT_PREOP_SUCCESS_WITH_CALLBACK;
}

NTSTATUS MiniUnload(FLT_FILTER_UNLOAD_FLAGS Flags) {
	DbgPrint("Driver Unloaded !! \n");
	return STATUS_SUCCESS;
}

const FLT_OPERATION_REGISTRATION Callbacks[] = { 
										{ IRP_MJ_CREATE, 0, MiniPreCreate, NULL },
										{ IRP_MJ_WRITE, 0, MiniPreWrite, NULL, NULL },
											{IRP_MJ_OPERATION_END}
										};
const FLT_REGISTRATION FilterRegistration = { sizeof(FLT_REGISTRATION),
												FLT_REGISTRATION_VERSION,
												0, NULL,
												Callbacks,
												MiniUnload,
												NULL, NULL, NULL, NULL,
												NULL, NULL, NULL, NULL
												 };

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath) {
	NTSTATUS status;
	status = FltRegisterFilter(DriverObject, &FilterRegistration, &MiniFilterHandle);
	if (NT_SUCCESS(status)) {
		status = FltStartFiltering(MiniFilterHandle);
		if (!NT_SUCCESS(status)) {
			DbgPrint(" MiniFilter canot start !!\n");
			FltUnregisterFilter(MiniFilterHandle);
		}
	}
	return status;
}