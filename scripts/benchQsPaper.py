import makeComparisonTable

benchmarks = [
        '../Benchmarks/Blasted_Real/blasted_case47.cnf',
        '../Benchmarks/Blasted_Real/blasted_case110.cnf',
        '../Benchmarks/V7/s820a_7_4.cnf',
        '../Benchmarks/V15/s820a_15_7.cnf',
        '../Benchmarks/V3/s1238a_3_2.cnf',
        '../Benchmarks/V3/s1196a_3_2.cnf',
        '../Benchmarks/V15/s832a_15_7.cnf',
        '../Benchmarks/Blasted_Real/blasted_case_1_b12_2.cnf',
        '../Benchmarks/Blasted_Real/blasted_squaring16.cnf',
        '../Benchmarks/Blasted_Real/blasted_squaring7.cnf',
        '../Benchmarks/70.sk_3_40.cnf',
        '../Benchmarks/ProcessBean.sk_8_64.cnf',
        '../Benchmarks/56.sk_6_38.cnf',
        '../Benchmarks/35.sk_3_52.cnf',
        '../Benchmarks/80.sk_2_48.cnf',
        '../Benchmarks/7.sk_4_50.cnf',
        '../Benchmarks/doublyLinkedList.sk_8_37.cnf',
        '../Benchmarks/19.sk_3_48.cnf',
        '../Benchmarks/29.sk_3_45.cnf',
        '../Benchmarks/isolateRightmost.sk_7_481.cnf',
        '../Benchmarks/17.sk_3_45.cnf',
        '../Benchmarks/81.sk_5_51.cnf',
        '../Benchmarks/LoginService2.sk_23_36.cnf',
        '../Benchmarks/sort.sk_8_52.cnf',
        #'../Benchmarks/parity.sk_11_11.cnf', # Crashes during qs sample checking
        '../Benchmarks/77.sk_3_44.cnf',
        '../Benchmarks/20.sk_1_51.cnf',
        '../Benchmarks/enqueueSeqSK.sk_10_42.cnf',
        '../Benchmarks/karatsuba.sk_7_41.cnf',
        '../Benchmarks/diagStencilClean.sk_41_36.cnf',
        '../Benchmarks/tutorial3.sk_4_31.cnf']

makeComparisonTable.main(benchmarks, 'comparison_table')
