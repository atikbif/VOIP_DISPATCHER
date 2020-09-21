#-------------------------------------------------
#
# Project created by QtCreator 2019-02-11T11:18:10
#
#-------------------------------------------------

QT       += core gui multimedia network printsupport sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

RC_ICONS = logo.ico
VERSION = 1.0.0.0

QMAKE_TARGET_COMPANY = modern technical solutions
QMAKE_TARGET_PRODUCT = Phone 1.1
QMAKE_TARGET_DESCRIPTION = VOIP Application
QMAKE_TARGET_COPYRIGHT = modern technical solutions

TARGET = GATE
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++17

DEFINES += _M_IX64
DEFINES += HAVE_CONFIG_H


#INCLUDEPATH += ../VOIP_DISPATCHER/speex-1.2.0/include/
#INCLUDEPATH += ../VOIP_DISPATCHER/speex-1.2.0/libspeex/
#INCLUDEPATH += ../VOIP_DISPATCHER/speex-1.2.0/src/
#INCLUDEPATH += ../VOIP_DISPATCHER/speex-1.2.0/win32/

INCLUDEPATH += ../VOIP_DISPATCHER/opus-1.3/include/
INCLUDEPATH += ../VOIP_DISPATCHER/opus-1.3/celt/
INCLUDEPATH += ../VOIP_DISPATCHER/opus-1.3/
INCLUDEPATH += ../VOIP_DISPATCHER/opus-1.3/silk/
INCLUDEPATH += ../VOIP_DISPATCHER/opus-1.3/silk/float
#INCLUDEPATH += ../opus/opus-1.3/silk/fixed
INCLUDEPATH += ../VOIP_DISPATCHER/opus-1.3/win32/



SOURCES += \
    audiotree.cpp \
    coloredsqlquerymodel.cpp \
    dialogdate.cpp \
    dialogvolumeconfig.cpp \
    group.cpp \
        main.cpp \
        mainwindow.cpp \
#        speex-1.2.0/libspeex/bits.c \
#        speex-1.2.0/libspeex/cb_search.c \
#        speex-1.2.0/libspeex/exc_5_64_table.c \
#        speex-1.2.0/libspeex/exc_5_256_table.c \
#        speex-1.2.0/libspeex/exc_8_128_table.c \
#        speex-1.2.0/libspeex/exc_10_16_table.c \
#        speex-1.2.0/libspeex/exc_10_32_table.c \
#        speex-1.2.0/libspeex/exc_20_32_table.c \
#        speex-1.2.0/libspeex/filters.c \
#        speex-1.2.0/libspeex/gain_table.c \
#        speex-1.2.0/libspeex/gain_table_lbr.c \
#        speex-1.2.0/libspeex/hexc_10_32_table.c \
#        speex-1.2.0/libspeex/hexc_table.c \
#        speex-1.2.0/libspeex/high_lsp_tables.c \
#        speex-1.2.0/libspeex/kiss_fft.c \
#        speex-1.2.0/libspeex/kiss_fftr.c \
#        speex-1.2.0/libspeex/lpc.c \
#        speex-1.2.0/libspeex/lsp.c \
#        speex-1.2.0/libspeex/lsp_tables_nb.c \
#        speex-1.2.0/libspeex/ltp.c \
#        speex-1.2.0/libspeex/modes.c \
#        speex-1.2.0/libspeex/modes_wb.c \
#        speex-1.2.0/libspeex/nb_celp.c \
#        speex-1.2.0/libspeex/quant_lsp.c \
#        speex-1.2.0/libspeex/sb_celp.c \
#        speex-1.2.0/libspeex/smallft.c \
#        speex-1.2.0/libspeex/speex.c \
#        speex-1.2.0/libspeex/speex_callbacks.c \
#        speex-1.2.0/libspeex/speex_header.c \
#        speex-1.2.0/libspeex/stereo.c \
#        speex-1.2.0/libspeex/vbr.c \
#        speex-1.2.0/libspeex/vq.c \
#        speex-1.2.0/libspeex/window.c \
    audioinputdevice.cpp \
    checksum.cpp \
    mp3recorder.cpp \
    point.cpp \
    projectconfig.cpp \
    sqldriver.cpp \
    sqlmanager.cpp \
    udpworker.cpp \
    udpcontroller.cpp \
    qcustomplot.cpp \
    audiooutputdevice.cpp \
    norwegianwoodstyle.cpp \
    dialogconfig.cpp \
    opus-1.3/celt/x86/celt_lpc_sse4_1.c \
    opus-1.3/celt/x86/pitch_sse.c \
    opus-1.3/celt/x86/pitch_sse2.c \
    opus-1.3/celt/x86/pitch_sse4_1.c \
    opus-1.3/celt/x86/vq_sse2.c \
    opus-1.3/celt/x86/x86_celt_map.c \
    opus-1.3/celt/x86/x86cpu.c \
    opus-1.3/celt/bands.c \
    opus-1.3/celt/celt.c \
    opus-1.3/celt/celt_decoder.c \
    opus-1.3/celt/celt_encoder.c \
    opus-1.3/celt/celt_lpc.c \
    opus-1.3/celt/cwrs.c \
    opus-1.3/celt/entcode.c \
    opus-1.3/celt/entdec.c \
    opus-1.3/celt/entenc.c \
    opus-1.3/celt/kiss_fft.c \
    opus-1.3/celt/laplace.c \
    opus-1.3/celt/mathops.c \
    opus-1.3/celt/mdct.c \
    opus-1.3/celt/modes.c \
    opus-1.3/celt/pitch.c \
    opus-1.3/celt/quant_bands.c \
    opus-1.3/celt/rate.c \
    opus-1.3/celt/vq.c \
    opus-1.3/silk/A2NLSF.c \
    opus-1.3/silk/ana_filt_bank_1.c \
    opus-1.3/silk/biquad_alt.c \
    opus-1.3/silk/bwexpander.c \
    opus-1.3/silk/bwexpander_32.c \
    opus-1.3/silk/check_control_input.c \
    opus-1.3/silk/CNG.c \
    opus-1.3/silk/code_signs.c \
    opus-1.3/silk/control_audio_bandwidth.c \
    opus-1.3/silk/control_codec.c \
    opus-1.3/silk/control_SNR.c \
    opus-1.3/silk/debug.c \
    opus-1.3/silk/dec_API.c \
    opus-1.3/silk/decode_core.c \
    opus-1.3/silk/decode_frame.c \
    opus-1.3/silk/decode_indices.c \
    opus-1.3/silk/decode_parameters.c \
    opus-1.3/silk/decode_pitch.c \
    opus-1.3/silk/decode_pulses.c \
    opus-1.3/silk/decoder_set_fs.c \
    opus-1.3/silk/enc_API.c \
    opus-1.3/silk/encode_indices.c \
    opus-1.3/silk/encode_pulses.c \
    opus-1.3/silk/gain_quant.c \
    opus-1.3/silk/HP_variable_cutoff.c \
    opus-1.3/silk/init_decoder.c \
    opus-1.3/silk/init_encoder.c \
    opus-1.3/silk/inner_prod_aligned.c \
    opus-1.3/silk/interpolate.c \
    opus-1.3/silk/lin2log.c \
    opus-1.3/silk/log2lin.c \
    opus-1.3/silk/LP_variable_cutoff.c \
    opus-1.3/silk/LPC_analysis_filter.c \
    opus-1.3/silk/LPC_fit.c \
    opus-1.3/silk/LPC_inv_pred_gain.c \
    opus-1.3/silk/NLSF2A.c \
    opus-1.3/silk/NLSF_decode.c \
    opus-1.3/silk/NLSF_del_dec_quant.c \
    opus-1.3/silk/NLSF_encode.c \
    opus-1.3/silk/NLSF_stabilize.c \
    opus-1.3/silk/NLSF_unpack.c \
    opus-1.3/silk/NLSF_VQ.c \
    opus-1.3/silk/NLSF_VQ_weights_laroia.c \
    opus-1.3/silk/NSQ.c \
    opus-1.3/silk/NSQ_del_dec.c \
    opus-1.3/silk/pitch_est_tables.c \
    opus-1.3/silk/PLC.c \
    opus-1.3/silk/process_NLSFs.c \
    opus-1.3/silk/quant_LTP_gains.c \
    opus-1.3/silk/resampler.c \
    opus-1.3/silk/resampler_down2.c \
    opus-1.3/silk/resampler_down2_3.c \
    opus-1.3/silk/resampler_private_AR2.c \
    opus-1.3/silk/resampler_private_down_FIR.c \
    opus-1.3/silk/resampler_private_IIR_FIR.c \
    opus-1.3/silk/resampler_private_up2_HQ.c \
    opus-1.3/silk/resampler_rom.c \
    opus-1.3/silk/shell_coder.c \
    opus-1.3/silk/sigm_Q15.c \
    opus-1.3/silk/sort.c \
    opus-1.3/silk/stereo_decode_pred.c \
    opus-1.3/silk/stereo_encode_pred.c \
    opus-1.3/silk/stereo_find_predictor.c \
    opus-1.3/silk/stereo_LR_to_MS.c \
    opus-1.3/silk/stereo_MS_to_LR.c \
    opus-1.3/silk/stereo_quant_pred.c \
    opus-1.3/silk/sum_sqr_shift.c \
    opus-1.3/silk/table_LSF_cos.c \
    opus-1.3/silk/tables_gain.c \
    opus-1.3/silk/tables_LTP.c \
    opus-1.3/silk/tables_NLSF_CB_NB_MB.c \
    opus-1.3/silk/tables_NLSF_CB_WB.c \
    opus-1.3/silk/tables_other.c \
    opus-1.3/silk/tables_pitch_lag.c \
    opus-1.3/silk/tables_pulses_per_block.c \
    opus-1.3/silk/VAD.c \
    opus-1.3/silk/VQ_WMat_EC.c \
    opus-1.3/silk/float/apply_sine_window_FLP.c \
    opus-1.3/silk/float/autocorrelation_FLP.c \
    opus-1.3/silk/float/burg_modified_FLP.c \
    opus-1.3/silk/float/bwexpander_FLP.c \
    opus-1.3/silk/float/corrMatrix_FLP.c \
    opus-1.3/silk/float/encode_frame_FLP.c \
    opus-1.3/silk/float/energy_FLP.c \
    opus-1.3/silk/float/find_LPC_FLP.c \
    opus-1.3/silk/float/find_LTP_FLP.c \
    opus-1.3/silk/float/find_pitch_lags_FLP.c \
    opus-1.3/silk/float/find_pred_coefs_FLP.c \
    opus-1.3/silk/float/inner_product_FLP.c \
    opus-1.3/silk/float/k2a_FLP.c \
    opus-1.3/silk/float/LPC_analysis_filter_FLP.c \
    opus-1.3/silk/float/LPC_inv_pred_gain_FLP.c \
    opus-1.3/silk/float/LTP_analysis_filter_FLP.c \
    opus-1.3/silk/float/LTP_scale_ctrl_FLP.c \
    opus-1.3/silk/float/noise_shape_analysis_FLP.c \
    opus-1.3/silk/float/pitch_analysis_core_FLP.c \
    opus-1.3/silk/float/process_gains_FLP.c \
    opus-1.3/silk/float/regularize_correlations_FLP.c \
    opus-1.3/silk/float/residual_energy_FLP.c \
    opus-1.3/silk/float/scale_copy_vector_FLP.c \
    opus-1.3/silk/float/scale_vector_FLP.c \
    opus-1.3/silk/float/schur_FLP.c \
    opus-1.3/silk/float/sort_FLP.c \
    opus-1.3/silk/float/warped_autocorrelation_FLP.c \
    opus-1.3/silk/float/wrappers_FLP.c \
    opus-1.3/silk/float/apply_sine_window_FLP.c \
    opus-1.3/silk/float/autocorrelation_FLP.c \
    opus-1.3/silk/float/burg_modified_FLP.c \
    opus-1.3/silk/float/bwexpander_FLP.c \
    opus-1.3/silk/float/corrMatrix_FLP.c \
    opus-1.3/silk/float/encode_frame_FLP.c \
    opus-1.3/silk/float/energy_FLP.c \
    opus-1.3/silk/float/find_LPC_FLP.c \
    opus-1.3/silk/float/find_LTP_FLP.c \
    opus-1.3/silk/float/find_pitch_lags_FLP.c \
    opus-1.3/silk/float/find_pred_coefs_FLP.c \
    opus-1.3/silk/float/inner_product_FLP.c \
    opus-1.3/silk/float/k2a_FLP.c \
    opus-1.3/silk/float/LPC_analysis_filter_FLP.c \
    opus-1.3/silk/float/LPC_inv_pred_gain_FLP.c \
    opus-1.3/silk/float/LTP_analysis_filter_FLP.c \
    opus-1.3/silk/float/LTP_scale_ctrl_FLP.c \
    opus-1.3/silk/float/noise_shape_analysis_FLP.c \
    opus-1.3/silk/float/pitch_analysis_core_FLP.c \
    opus-1.3/silk/float/process_gains_FLP.c \
    opus-1.3/silk/float/regularize_correlations_FLP.c \
    opus-1.3/silk/float/residual_energy_FLP.c \
    opus-1.3/silk/float/scale_copy_vector_FLP.c \
    opus-1.3/silk/float/scale_vector_FLP.c \
    opus-1.3/silk/float/schur_FLP.c \
    opus-1.3/silk/float/sort_FLP.c \
    opus-1.3/silk/float/warped_autocorrelation_FLP.c \
    opus-1.3/silk/float/wrappers_FLP.c \
    opus-1.3/silk/x86/NSQ_del_dec_sse4_1.c \
   opus-1.3/silk/x86/NSQ_sse4_1.c \
    opus-1.3/silk/x86/VAD_sse4_1.c \
    opus-1.3/silk/x86/VQ_WMat_EC_sse4_1.c \
    opus-1.3/silk/x86/x86_silk_map.c \
    opus-1.3/src/analysis.c \
    opus-1.3/src/mapping_matrix.c \
    opus-1.3/src/mlp.c \
    opus-1.3/src/mlp_data.c \
    opus-1.3/src/opus.c \
    opus-1.3/src/opus_decoder.c \
    opus-1.3/src/opus_encoder.c \
    opus-1.3/src/opus_multistream.c \
    opus-1.3/src/opus_multistream_decoder.c \
    opus-1.3/src/opus_multistream_encoder.c \
    opus-1.3/src/opus_projection_decoder.c \
    opus-1.3/src/opus_projection_encoder.c \
    opus-1.3/src/repacketizer.c

HEADERS += \
    audiotree.h \
    coloredsqlquerymodel.h \
    dialogdate.h \
    dialogvolumeconfig.h \
    group.h \
        mainwindow.h \
    audioinputdevice.h \
    checksum.h \
    mp3recorder.h \
    point.h \
    projectconfig.h \
    sqldriver.h \
    sqlmanager.h \
    udpworker.h \
    udpcontroller.h \
    qcustomplot.h \
    audiooutputdevice.h \
    norwegianwoodstyle.h \
    dialogconfig.h \
    opus-1.3/celt/x86/celt_lpc_sse.h \
    opus-1.3/celt/x86/pitch_sse.h \
    opus-1.3/celt/x86/vq_sse.h \
    opus-1.3/celt/x86/x86cpu.h \
    opus-1.3/celt/_kiss_fft_guts.h \
    opus-1.3/celt/arch.h \
    opus-1.3/celt/bands.h \
    opus-1.3/celt/celt.h \
    opus-1.3/celt/celt_lpc.h \
    opus-1.3/celt/cpu_support.h \
    opus-1.3/celt/cwrs.h \
    opus-1.3/celt/ecintrin.h \
    opus-1.3/celt/entcode.h \
    opus-1.3/celt/entdec.h \
    opus-1.3/celt/entenc.h \
    opus-1.3/celt/fixed_debug.h \
    opus-1.3/celt/fixed_generic.h \
    opus-1.3/celt/float_cast.h \
    opus-1.3/celt/kiss_fft.h \
    opus-1.3/celt/laplace.h \
    opus-1.3/celt/mathops.h \
    opus-1.3/celt/mdct.h \
    opus-1.3/celt/mfrngcod.h \
    opus-1.3/celt/modes.h \
    opus-1.3/celt/os_support.h \
    opus-1.3/celt/pitch.h \
    opus-1.3/celt/quant_bands.h \
    opus-1.3/celt/rate.h \
    opus-1.3/celt/stack_alloc.h \
    opus-1.3/celt/static_modes_fixed.h \
    opus-1.3/celt/static_modes_fixed_arm_ne10.h \
    opus-1.3/celt/static_modes_float.h \
    opus-1.3/celt/static_modes_float_arm_ne10.h \
    opus-1.3/celt/vq.h \
    opus-1.3/include/opus.h \
    opus-1.3/include/opus_custom.h \
    opus-1.3/include/opus_defines.h \
    opus-1.3/include/opus_multistream.h \
    opus-1.3/include/opus_projection.h \
    opus-1.3/include/opus_types.h \
    opus-1.3/silk/fixed/main_FIX.h \
    opus-1.3/silk/fixed/structs_FIX.h \
    opus-1.3/silk/float/main_FLP.h \
    opus-1.3/silk/float/SigProc_FLP.h \
    opus-1.3/silk/float/structs_FLP.h \
    opus-1.3/silk/API.h \
    opus-1.3/silk/control.h \
    opus-1.3/silk/debug.h \
    opus-1.3/silk/define.h \
    opus-1.3/silk/errors.h \
    opus-1.3/silk/Inlines.h \
    opus-1.3/silk/MacroCount.h \
    opus-1.3/silk/MacroDebug.h \
    opus-1.3/silk/macros.h \
    opus-1.3/silk/main.h \
    opus-1.3/silk/NSQ.h \
    opus-1.3/silk/pitch_est_defines.h \
    opus-1.3/silk/PLC.h \
    opus-1.3/silk/resampler_private.h \
    opus-1.3/silk/resampler_rom.h \
    opus-1.3/silk/resampler_structs.h \
    opus-1.3/silk/SigProc_FIX.h \
    opus-1.3/silk/structs.h \
    opus-1.3/silk/tables.h \
    opus-1.3/silk/tuning_parameters.h \
    opus-1.3/silk/typedef.h \
    opus-1.3/silk/float/main_FLP.h \
    opus-1.3/silk/float/SigProc_FLP.h \
    opus-1.3/silk/float/structs_FLP.h \
    opus-1.3/silk/fixed/main_FIX.h \
    opus-1.3/silk/fixed/structs_FIX.h \
    opus-1.3/silk/float/main_FLP.h \
    opus-1.3/silk/float/SigProc_FLP.h \
    opus-1.3/silk/float/structs_FLP.h \
    opus-1.3/silk/x86/main_sse.h \
    opus-1.3/silk/x86/SigProc_FIX_sse.h \
    opus-1.3/src/analysis.h \
    opus-1.3/src/mapping_matrix.h \
    opus-1.3/src/mlp.h \
    opus-1.3/src/opus_private.h \
    opus-1.3/src/tansig_table.h \
    opus-1.3/win32/config.h \
    wav_example.h

FORMS += \
    dialogdate.ui \
    dialogvolumeconfig.ui \
        mainwindow.ui \
    dialogconfig.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    styles.qrc
