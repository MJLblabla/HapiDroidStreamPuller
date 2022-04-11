package com.hapi.srtmuxer.ts

import android.media.MediaFormat
import com.hapi.srtmuxer.ts.data.Service
import com.hapi.srtmuxer.ts.data.ServiceInfo
import com.hapi.srtmuxer.ts.data.Stream
import com.hapi.srtmuxer.ts.packets.Pat
import com.hapi.srtmuxer.ts.packets.Pes
import com.hapi.srtmuxer.ts.packets.Pmt
import com.hapi.srtmuxer.ts.packets.Sdt
import com.hapi.srtmuxer.ts.utils.MuxerConst
import com.hapi.srtmuxer.ts.utils.TSConst

import java.nio.ByteBuffer
import java.util.*
import kotlin.random.Random

class TSMuxer(
    private val muxerListener: IMuxerListener,
    firstServiceInfo: ServiceInfo? = null,
    initialStreams: List<String>? = null
) {
    private val tsServices = mutableListOf<Service>()
    private val tsPes = mutableListOf<Pes>()

    private val tsId = Random.nextInt(Byte.MIN_VALUE.toInt(), Byte.MAX_VALUE.toInt()).toShort()

    private var pat = Pat(
        muxerListener,
        tsServices,
        tsId,
        packetCount = 0
    )
    private var sdt = Sdt(
        muxerListener,
        tsServices,
        tsId,
        packetCount = 0
    )

    init {
        if (initialStreams != null) {
            require(firstServiceInfo != null) { "If streams are specified, a service info must be specified too" }
        }
        firstServiceInfo?.let { addService(it) }
        initialStreams?.let { addStreams(tsServices[0], it) }
    }

    /**
     * Encode a frame to MPEG-TS format.
     * Each audio frames and each video key frames must come with an extra buffer containing sps, pps,...
     * @param frame frame to mux
     * @param streamPid Pid of frame stream. Throw a NoSuchElementException if streamPid refers to an unknown stream
     */
    fun encode(frame: Frame, streamPid: Short) {
        when (frame.mimeType) {
            MediaFormat.MIMETYPE_VIDEO_AVC -> {
                // Copy sps & pps before buffer
                if (frame.isKeyFrame) {
                    if (frame.extra == null) {
                        throw MissingFormatArgumentException("Missing extra for AVC")
                    }
                    val buffer =
                        ByteBuffer.allocate(
                            6 + frame.extra.limit() + frame.buffer.limit()
                        )
                    buffer.putInt(0x00000001)
                    buffer.put(0x09.toByte())
                    buffer.put(0xf0.toByte())
                    buffer.put(frame.extra)
                    buffer.put(frame.buffer)
                    buffer.rewind()
                    frame.buffer = buffer
                }
            }
            MediaFormat.MIMETYPE_VIDEO_HEVC -> {
                // Copy sps & pps & vps before buffer
                if (frame.isKeyFrame) {
                    if (frame.extra == null) {
                        throw MissingFormatArgumentException("Missing extra for HEVC")
                    }
                    val buffer =
                        ByteBuffer.allocate(
                            7 + frame.extra.limit() + frame.buffer.limit()
                        )
                    buffer.putInt(0x00000001)
                    buffer.put(0x46.toByte())
                    buffer.put(0x01.toByte())
                    buffer.put(0x50.toByte())
                    buffer.put(frame.extra)
                    buffer.put(frame.buffer)
                    buffer.rewind()
                    frame.buffer = buffer
                }
            }
            MediaFormat.MIMETYPE_AUDIO_AAC -> {
                // Copy ADTS
                if (frame.extra == null) {
                    throw MissingFormatArgumentException("Missing extra ADTS for AAC")
                }
                val buffer =
                    ByteBuffer.allocate(frame.buffer.limit() + frame.extra.limit())
                frame.extra.let { buffer.put(it) }
                buffer.put(frame.buffer)
                buffer.rewind()
                frame.buffer = buffer
            }
            else -> TODO("Format not yet implemented")
        }

        synchronized(this) {
            generateStreams(frame, getPes(streamPid))
        }
    }

    /**
     * Generate MPEG-TS table and elementary stream from the frame
     * @param pes Pes containing infos on the stream
     * @param frame frame to mux
     */
    private fun generateStreams(frame: Frame, pes: Pes) {
        retransmitPsi(frame.mimeType.isVideo() and frame.isKeyFrame)
        pes.write(frame)
    }

    /**
     * Manage table retransmission
     * @param forcePat Force to remit a PAT. Set to true on video key frame.
     */
    private fun retransmitPsi(forcePat: Boolean) {
        var sendSdt = false
        var sendPat = false

        sdt.packetCount += 1
        if (sdt.packetCount == MuxerConst.SDT_PACKET_PERIOD) {
            sdt.packetCount = 0
            sendSdt = true
        }

        pat.packetCount += 1
        if ((pat.packetCount == MuxerConst.PAT_PACKET_PERIOD) || forcePat) {
            pat.packetCount = 0
            sendPat = true
        }

        if (sendSdt) {
            sendSdt()
        }
        if (sendPat) {
            sendPat()
            sendPmts()
        }
    }

    private fun upgradePat() {
        pat.versionNumber = (pat.versionNumber + 1).toByte()
        sendPat()
    }

    private fun sendPat() {
        pat.write()
    }


    private fun sendPmt(service: Service) {
        service.pmt?.write() ?: throw UnsupportedOperationException("PMT must not be null")
    }

    private fun sendPmts() {
        tsServices.filter { it.pmt != null }.forEach {
            it.pmt?.write() ?: throw UnsupportedOperationException("PMT must not be null")
        }
    }

    private fun upgradeSdt() {
        sdt.versionNumber = (sdt.versionNumber + 1).toByte()
        sendSdt()
    }

    private fun sendSdt() {
        sdt.write()
    }

    /**
     * Get registered services list
     * @return list of registered services
     */
    fun getService(): List<ServiceInfo> {
        return tsServices.map { it.info }
    }

    /**
     * Register a new service. Service will be valid (tables will be emitted) as soon as streams will be added.
     * @param serviceInfo new service to add to service list
     */
    fun addService(serviceInfo: ServiceInfo) {
        require(tsServices.none { it.info == serviceInfo }) { "Service already exists" }

        tsServices.add(Service(serviceInfo))
    }

    /**
     * Remove a service and its streams
     * @param serviceInfo service info of service to remove
     */
    fun removeService(serviceInfo: ServiceInfo) = removeService(getService(serviceInfo))

    /**
     * Remove a service and its streams
     * @param service service to remove
     */
    private fun removeService(service: Service) {
        require(tsServices.contains(service)) { "Unknown service" }
        tsServices.remove(service)

        if (service.streams.isNotEmpty()) {
            removeStreams(service, service.streams)
        }

        upgradeSdt()
        upgradePat()
    }

    /**
     * Add streams for a service
     * @param serviceInfo service where to add streams
     * @param streamsMimeType list of mime type
     * @return ordered list of stream id
     */
    fun addStreams(serviceInfo: ServiceInfo, streamsMimeType: List<String>): List<Short> {
        return addStreams(getService(serviceInfo), streamsMimeType)
    }

    /**
     * Add streams for a service
     * @param service service where to add streams
     * @param streamsMimeType list of mime type
     * @return list of corresponding PES
     */
    private fun addStreams(service: Service, streamsMimeType: List<String>): List<Short> {
        require(tsServices.contains(service)) { "Unknown service" }

        val isNewService = service.pmt == null

        val newStreams = mutableListOf<Stream>()
        streamsMimeType.forEach {
            val stream = Stream(it, getNewPid())
            newStreams.add(stream)
            service.streams.add(stream)
        }

        service.pcrPid = try {
            service.streams.first { it.mimeType.isVideo() }.pid
        } catch (e: NoSuchElementException) {
            service.streams[0].pid
        }

        // Prepare tables
        service.pmt = service.pmt?.apply {
            versionNumber = (versionNumber + 1).toByte()
            streams = service.streams
        } ?: Pmt(muxerListener, service, service.streams, getNewPid())

        // Init PES
        newStreams.forEach {
            Pes(
                muxerListener,
                it,
                service.pcrPid == it.pid,
            ).run { tsPes.add(this) }
        }

        // Send tables
//        sendPmt(service)
//        if (isNewService) {
//            upgradeSdt()
//            upgradePat()
//        }

        return newStreams.map { it.pid }
    }

    /**
     * Remove streams from service. If you want to remove all streams from a service,
     * use {@link removeService} instead.
     * @param serviceInfo service info
     * @param streamsPid list of streams to remove
     */
    fun removeStreams(serviceInfo: ServiceInfo, streamsPid: List<Short>) =
        removeStreams(getService(serviceInfo), streamsPid.map { getStream(it) })

    /**
     * Remove streams from service. If you want to remove all streams from a service,
     * use {@link removeService} instead.
     * @param service service
     * @param streams list of streams to remove
     */
    private fun removeStreams(service: Service, streams: List<Stream>) {
        service.streams.forEach {
            tsPes.remove(getPes(it.pid))
        }
        service.streams.removeAll(streams)

        service.pmt?.let {
            it.versionNumber = (it.versionNumber + 1).toByte()
            it.streams = service.streams
        }

        sendPmt(service)
    }

    /**
     * Clear internal parameters
     */
    fun stop() {
        tsPes.clear()
        tsServices.clear()
    }

    /**
     * Get a new Pid between [0x00010 and 0x1FFE]. Call it when you need a new PID for a PMT, streams,...
     * @return a short value
     */
    private fun getNewPid(): Short {
        val currentPids =
            tsServices.flatMap { it.streams }.map { it.pid } + tsServices.filter { it.pmt != null }
                .map { it.pmt?.pid }

        for (i in TSConst.BASE_PID until 0x1FFA) {
            if (!currentPids.contains(i.toShort())) {
                return i.toShort()
            }
        }

        throw IndexOutOfBoundsException("No empty PID left")
    }

    /**
     * Get PES from stream Pid
     * @param pid stream pid
     * @return PES
     */
    private fun getPes(pid: Short): Pes {
        return tsPes.first { it.stream.pid == pid }
    }

    /**
     * Get list of streams from a MimeType
     * @param mimeType stream mimetype
     * @return list of streams with same MimeType
     */
    fun getStreams(mimeType: String): List<Stream> {
        return tsServices.flatMap { it.streams }.filter { it.mimeType == mimeType }
    }

    /**
     * Get stream from Pid
     * @param pid stream pid
     * @return streams of Pid
     */
    private fun getStream(pid: Short): Stream {
        return tsServices.flatMap { it.streams }.first { it.pid == pid }
    }

    /**
     * Get Service from ServiceInfo
     * @param serviceInfo service info
     * @return Service
     */
    private fun getService(serviceInfo: ServiceInfo): Service {
        return tsServices.first { it.info == serviceInfo }
    }

}