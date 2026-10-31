FROM python:3.14.3-trixie

SHELL ["/bin/bash", "-c"]

ARG DEBIAN_FRONTEND=noninteractive

ARG IDF_PATH=/builder/ESP8266_RTOS_SDK

ENV PATH="$PATH:/builder/xtensa-lx106-elf/bin"
ENV PYTHONPATH="$PYTHONPATH:/usr/lib/python3/dist-packages"

WORKDIR /builder

USER root
RUN \
	apt-get update && \
	apt-get install -y \
		gcc \
		git \
		wget \
		make \
		cmake \
		libncurses-dev \
		flex \
		bison \
		python-is-python3 \
		python3-setuptools \
		python3-pip \
		python3-serial \
		esptool \
		ninja-build && \
	apt-get clean && \
	rm -f -r '/var/lib/apt/' && \
	rm -f -r '/var/cache/apt/'

ENV LANG=en_US.utf8

RUN wget -O- https://dl.espressif.com/dl/xtensa-lx106-elf-linux64-1.22.0-100-ge567ec7-5.2.0.tar.gz | \
	tar -zxf -

COPY . .

RUN set -ex && \
	python -m pip install --upgrade --user pip && \
	python -m pip install --user -r ./ESP8266_RTOS_SDK/requirements.txt

RUN set -ex && \
		python ./idf.py fullclean && \
		python ./idf.py build && \
		echo "::set-output name=status::success"

RUN set -ex && \
	for chip in 'esp8266' 'esp32' 'esp32c3' 'esp32s3'; \
	do \
		bootloader_addr=0x0; \
		if [ $chip == 'esp32']; then \
			bootloader_addr=0x1000; \
		fi; \
		esptool \
			--chip $chip \
			merge_bin  \
			-o build/wireless_esp_dap_full_$chip.bin \
			$bootloader_addr build/bootloader/bootloader.bin \
			0x8000 build/partition_table/partition-table.bin \
			0x10000 build/wireless_esp_dap.bin ;\
	done

EXPOSE 8000

CMD ["python", "-m", "http.server", "8000"]
